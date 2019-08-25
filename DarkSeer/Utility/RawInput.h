#pragma once
#include <Windows.h>
#include <stdint.h>
#include <numeric>
#include <tuple>
#include "Math.h"
inline namespace JobScheduler
{
        inline namespace Interface
        {
                struct JobInternal;
        }
        inline namespace Globals
        {
                inline DWORD                     g_tls_access_value = TlsAlloc();
                inline unsigned                  g_num_threads      = std::thread::hardware_concurrency();
                inline std::vector<std::thread>  g_worker_threads;
                inline volatile bool             g_worker_thread_active = true;
                inline thread_local std::mt19937 g_thread_local_rng_engine;

                inline constexpr unsigned CACHE_LINE_SIZE    = std::hardware_destructive_interference_size;
                inline constexpr unsigned MAX_JOBS_PER_FRAME = 8192U;
        } // namespace Globals
        inline namespace Utility
        {
                inline unsigned GenerateRandomNumber(unsigned min, unsigned max)
                {
                        return (g_thread_local_rng_engine() % (max - min)) + min;
                }
        } // namespace Utility
        inline namespace Interface
        {
                using JobFunction = void (*)(JobInternal*);
                struct alignas(64) JobInternal
                {
                        JobFunction           function;
                        JobInternal*          parent;
                        volatile long         unfinished_jobs;
                        static constexpr auto PADDING_SIZE =
                            CACHE_LINE_SIZE - sizeof(function) - sizeof(parent) - sizeof(unfinished_jobs);
                        char padding[PADDING_SIZE];
                };
                template <unsigned MAX_JOBS>
                struct JobAllocator
                {
                        static constexpr uint64_t MAX_JOBS = saturatePowerOf2(MAX_JOBS);
                        static constexpr uint64_t MASK     = MAX_JOBS - 1;
                        alignas(8) JobInternal* job_buffer = 0;
                        alignas(8) uint64_t allocated_jobs = 0;
                        JobAllocator()
                        {}
                        void Initialize()
                        {
                                job_buffer = (JobInternal*)_aligned_malloc(sizeof(JobInternal) * MAX_JOBS, 64);
                        }
                        JobInternal* Allocate()
                        {
                                return job_buffer + (allocated_jobs++ & MASK);
                        }
                        void Release()
                        {
                                _aligned_free(job_buffer);
                        }
                        ~JobAllocator()
                        {}
                };

                template <unsigned MAX_JOBS>
                struct JobQueue
                {
                        static constexpr auto MAX_JOBS = saturatePowerOf2(MAX_JOBS);
                        static constexpr auto MASK     = MAX_JOBS - 1;
                        JobInternal**         m_jobs;
                        alignas(8) volatile int64_t m_bottom;
                        alignas(8) volatile int64_t m_top;
                        JobQueue()
                        {}
                        void Initialize() volatile
                        {
                                m_jobs   = (JobInternal**)_aligned_malloc(sizeof(JobInternal*) * MAX_JOBS, 8);
                                m_bottom = 0;
                                m_top    = 0;
                        }
                        void Shutdown() volatile
                        {
                                _aligned_free(m_jobs);
                        }
                        JobQueue(JobQueue&& other) noexcept
                        {
                                m_bottom     = other.m_bottom;
                                m_top        = other.m_top;
                                m_jobs       = other.m_jobs;
                                other.m_jobs = 0;
                        }
                        ~JobQueue()
                        {}
                        void Push(JobInternal* job) volatile
                        {
                                long b           = m_bottom;
                                m_jobs[b & MASK] = job;

                                // ensure the job is written before b+1 is published to other threads.
                                // on x86/64, a compiler barrier is enough.
                                std::atomic_thread_fence(std::memory_order_seq_cst);

                                m_bottom = b + 1;
                        }
                        JobInternal* Pop(void) volatile
                        {
                                long b = m_bottom - 1;
                                InterlockedExchange64(&m_bottom, b);

                                long t = m_top;
                                if (t <= b)
                                {
                                        // non-empty queue
                                        JobInternal* job = m_jobs[b & MASK];
                                        if (t != b)
                                        {
                                                // there's still more than one item left in the queue
                                                return job;
                                        }

                                        // this is the last item in the queue
                                        if (InterlockedCompareExchange64(&m_top, t + 1, t) != t)
                                        {
                                                // failed race against steal operation
                                                job = nullptr;
                                        }

                                        m_bottom = t + 1;
                                        return job;
                                }
                                else
                                {
                                        // deque was already empty
                                        m_bottom = t;
                                        return nullptr;
                                }
                        }
                        JobInternal* Steal(void) volatile
                        {
                                long t = m_top;

                                // ensure that top is always read before bottom.
                                // loads will not be reordered with other loads on x86, so a compiler barrier is enough.
                                std::atomic_thread_fence(std::memory_order_seq_cst);

                                long b = m_bottom;
                                if (t < b)
                                {
                                        // non-empty queue
                                        JobInternal* job = m_jobs[t & MASK];

                                        // the interlocked function serves as a compiler barrier, and guarantees that the read
                                        // happens before the CAS.
                                        if (InterlockedCompareExchange64(&m_top, t + 1, t) != t)
                                        {
                                                // a concurrent steal or pop operation removed an element from the deque in the
                                                // meantime.
                                                return nullptr;
                                        }

                                        return job;
                                }
                                else
                                {
                                        // empty queue
                                        return nullptr;
                                }
                        }
                };
        } // namespace Interface
        inline namespace Globals
        {
                inline thread_local JobAllocator<MAX_JOBS_PER_FRAME> g_thread_local_job_allocator_temp;
                inline thread_local JobAllocator<MAX_JOBS_PER_FRAME> g_thread_local_job_allocator_static;

                alignas(8) volatile inline JobQueue<MAX_JOBS_PER_FRAME>* volatile g_job_queues;
        } // namespace Globals
        inline namespace Internal
        {
                inline auto& GetWorkerThreadQueue()
                {
                        return g_job_queues[(unsigned)TlsGetValue(g_tls_access_value)];
                }
                inline void Run(JobInternal* job)
                {
                        auto& queue = GetWorkerThreadQueue();
                        queue.Push(job);
                }
                inline JobInternal* AllocateJob()
                {
                        return g_thread_local_job_allocator_temp.Allocate();
                }
                inline JobInternal* CreateJob(JobFunction function)
                {
                        JobInternal* job     = AllocateJob();
                        job->function        = function;
                        job->parent          = nullptr;
                        job->unfinished_jobs = 1;
                        return job;
                }
                inline JobInternal* CreateJobAsChild(JobInternal* parent, JobFunction function)
                {
                        InterlockedIncrement(&parent->unfinished_jobs);
                        JobInternal* job     = AllocateJob();
                        job->function        = function;
                        job->parent          = parent;
                        job->unfinished_jobs = 1;
                        return job;
                }
                inline bool HasJobCompleted(JobInternal* job)
                {
                        return job->unfinished_jobs == 0;
                }
                inline void Finish(JobInternal* job)
                {
                        const long unfinished_jobs = InterlockedDecrement(&job->unfinished_jobs);
                        if (unfinished_jobs == 0 && job->parent)
                        {
                                Finish(job->parent);
                        }
                }
                inline void Execute(JobInternal* job)
                {
                        job->function(job);
                        Finish(job);
                }
                inline JobInternal* GetJob()
                {
                        auto&        queue = GetWorkerThreadQueue();
                        JobInternal* job   = queue.Pop();
                        if (!job)
                        {
                                // this is not a valid job because our own queue is empty, so try stealing from some other queue
                                unsigned int randomIndex = GenerateRandomNumber(0, g_num_threads);
                                auto&        stealQueue  = g_job_queues[randomIndex];
                                if (&stealQueue == &queue)
                                {
                                        // don't try to steal from ourselves
                                        Sleep(0);
                                        return nullptr;
                                }

                                JobInternal* stolenJob = stealQueue.Steal();
                                if (!stolenJob)
                                {
                                        // we couldn't steal a job from the other queue either, so we just yield our time slice
                                        // for now
                                        Sleep(0);
                                        return nullptr;
                                }

                                return stolenJob;
                        }
                        return job;
                }
                inline void Wait(JobInternal* job)
                {
                        while (!HasJobCompleted(job))
                        {
                                JobInternal* nextJob = GetJob();
                                if (nextJob)
                                {
                                        Execute(nextJob);
                                }
                                // std::atomic_thread_fence(std::memory_order_seq_cst);
                        }
                }
                inline void WorkerThreadMain(unsigned index)
                {
                        TlsSetValue(g_tls_access_value, (LPVOID)index);
                        while (g_worker_thread_active)
                        {
                                JobInternal* job = GetJob();
                                if (job)
                                {
                                        Execute(job);
                                }
                        }
                }
        } // namespace Internal
} // namespace JobScheduler

inline namespace RawInputE
{
        inline namespace Enums
        {
#undef ENUM
#define ENUM(E, V) INPUT_##E,
                // dummy enum to get number of mouse signatures
                enum class DummyEnum
                {
#include "MOUSE_SCANCODES.ENUM"
                        NUM_MOUSE_SIGNATURES
                };
#undef ENUM
#define ENUM(E, V) INPUT_##E,
                enum ButtonSignature : uint16_t
                {
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"

#include "MOUSE_SCANCODES.ENUM"
                        INPUT_NUM_SCANCODE_SIGNATURES
                };
#undef ENUM
                constexpr uint8_t  INPUT_NUM_MOUSE_SCANCODES = (uint8_t)DummyEnum::NUM_MOUSE_SIGNATURES;
                constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES =
                    INPUT_NUM_SCANCODE_SIGNATURES - INPUT_NUM_MOUSE_SCANCODES;
                constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODES = INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES / 3;
#define ENUM(E, V) #E,
                constexpr const char* buttonSignatureToString[INPUT_NUM_SCANCODE_SIGNATURES + 1]{
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"

#include "MOUSE_SCANCODES.ENUM"
                    "INPUT_NUM_SCANCODE_SIGNATURE_ENUMS"};
#undef ENUM

#define ENUM(E, V) bool INPUT_##E : 1;
                struct PressState
                {
                        static constexpr auto _SZ64 = 48 /*sizeof(PressState)*/ / sizeof(uint64_t);
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"

#include "MOUSE_SCANCODES.ENUM"

                        void KeyDown(ButtonSignature button)
                        {
                                uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                                uint64_t mask                     = 1ULL << ((uint64_t)button & (64ULL - 1ULL));
                                pressStateAlias[button >> 6] |= mask;
                        }
                        void KeyUp(ButtonSignature button)
                        {
                                uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                                uint64_t mask                     = 1ULL << ((uint64_t)button & (64ULL - 1ULL));
                                pressStateAlias[button >> 6] &= ~mask;
                        }
                };
#undef ENUM
                // must be size 2 bytes to use the Transition state to store scroll delta if scroll button signature is set
                enum TransitionState : int8_t
                {
                        INPUT_transitionStateDown,
                        INPUT_transitionStateUp
                };
                constexpr const char* transitionStateToString[2]{"down", "up"};
        } // namespace Enums

        struct alignas(64) InputFrameE
        {
                PressState             m_pressState;      //				48	B
                std::tuple<long, long> m_mouseDeltas;     //				8	B
                ButtonSignature        m_buttonSignature; // buttonId //	2	B
                TransitionState        m_transitionState; // up or down //	1	B
                char                   m_padding[5];
        };
        // constexpr auto _SizeOfPressState      = sizeof(PressState);
        // constexpr auto _SizeofInputFrameE     = sizeof(InputFrameE);
        // constexpr auto _SizeofButtonSignature = sizeof(ButtonSignature);
        // constexpr auto _SizeofTransitionState = sizeof(TransitionState);

        struct RawInputBufferE
        {
            public:
                PressState m_prevPressState;

            private:
                static constexpr unsigned long long m_SZ_in          = saturatePowerOf2(10000ULL);
                static constexpr auto               m_lcm            = std::lcm(sizeof(InputFrameE), sizeof(__m256i));
                static constexpr auto               m_frameChunkSize = m_lcm / sizeof(InputFrameE);
                static constexpr auto               m_256ChunkSize   = m_lcm / sizeof(__m256i);
                // saturate SZ and SZ_M256 to their least common multiple sizes
                static constexpr auto m_SZ      = Math::DivideRoundUp(m_SZ_in, m_frameChunkSize) * m_frameChunkSize;
                static constexpr auto m_SZ_M256 = (m_SZ * sizeof(InputFrameE)) / sizeof(__m256i);

                static constexpr unsigned m_NUM_SWAP_BUFFERS = saturatePowerOf2(2);
                // m_NUM_SWAP_BUFFERS must be power of 2, swap() uses mersenne prime modulation


                //static constexpr auto MAX_JOBS = saturatePowerOf2(m_SZ);
                //static constexpr auto MASK     = MAX_JOBS - 1;
                //JobInternal**         m_jobs;
                //alignas(8) volatile int64_t m_bottom;
                //alignas(8) volatile int64_t m_top;


                volatile __m256i*     m_inputFramesM256;
                volatile InputFrameE* m_inputFrames;

                volatile uint64_t m_bufferSizes[m_NUM_SWAP_BUFFERS];
                unsigned          m_readSwapBufferIndex  = 0;
                unsigned          m_writeSwapBufferIndex = 1;

                void swap()
                {
                        m_readSwapBufferIndex  = m_writeSwapBufferIndex;
                        m_writeSwapBufferIndex = (m_writeSwapBufferIndex + 1ULL) & (m_NUM_SWAP_BUFFERS - 1);
                }

                bool try_push(const InputFrameE& InputFrame)
                {
                        const auto bufferWriteIndex = m_bufferSizes[m_writeSwapBufferIndex];

                        if (bufferWriteIndex == m_SZ)
                                return false;

                        auto&      inputFrameSz = const_cast<uint64_t&>(m_bufferSizes[m_writeSwapBufferIndex]);
                        const auto local_offset = (m_writeSwapBufferIndex * m_SZ) + inputFrameSz;

                        auto& inputFramePrev = const_cast<InputFrameE&>(m_inputFrames[local_offset - 1]);
                        auto& inputFrameCurr = const_cast<InputFrameE&>(m_inputFrames[local_offset]);
                        // mouse move		:	push_back or concat_back
                        if (!InputFrame.m_buttonSignature)
                        {
                                // array is empty						:		always push_back
                                if (!inputFrameSz)
                                {
                                        inputFrameCurr = InputFrame;
                                        inputFrameSz++;
                                }
                                // last push was a button press			:		push_back
                                else if (inputFramePrev.m_buttonSignature)
                                {
                                        inputFrameCurr = InputFrame;
                                        inputFrameSz++;
                                }
                                // last push was a mouse move			:		concat_back
                                else
                                {
                                        inputFramePrev.m_mouseDeltas += InputFrame.m_mouseDeltas;
                                }
                        }
                        // button press		:	always push_back
                        else
                        {
                                inputFrameCurr = InputFrame;
                                inputFrameSz++;
                        }
                        return true;
                }

            public:
                // single consumer functions:
                void process_writes()
                {
                        const unsigned num_to_read  = m_bufferSizes[m_readSwapBufferIndex];
                        const unsigned local_offset = m_readSwapBufferIndex * m_SZ;

                        for (unsigned i = 0; i < num_to_read; i++)
                        {
                                const auto& thisFrame = const_cast<InputFrameE&>(m_inputFrames[local_offset + i]);

                                if (thisFrame.m_pressState.INPUT_a && thisFrame.m_pressState.INPUT_s &&
                                    thisFrame.m_pressState.INPUT_d)
                                        std::cout << "holding asd\n\n";

                                // const auto [x, y]     = thisFrame.m_mouseDeltas;
                                // std::cout << "x:\t" << x << std::endl;
                                // std::cout << "y:\t" << y << std::endl;
                                // std::cout << "buttonSignature:\t" << buttonSignatureToString[thisFrame.m_buttonSignature]
                                //          << std::endl;
                                // if (thisFrame.m_buttonSignature != INPUT_mouseScrollVertical)
                                //        std::cout << "transitionState:\t"
                                //                  << transitionStateToString[thisFrame.m_transitionState] << std::endl;
                                // else
                                //        std::cout << "scrollDelta:\t" << ((short)thisFrame.m_transitionState) << std::endl;
                                // std::cout << std::endl;
                        }
                        m_bufferSizes[m_readSwapBufferIndex] = 0;
                        Sleep(30);
                }

                // single producer functions:
                void write(InputFrameE& rawInputFrame)
                {
                        /* std::cout << "x:\t" << std::get<0>(rawInputFrame.m_mouseDeltas) << std::endl;
                         std::cout << "y:\t" << std::get<1>(rawInputFrame.m_mouseDeltas) << std::endl;
                         std::cout << "buttonFlag:\t" << buttonSignatureToString[rawInputFrame.m_buttonSignature] << std::endl;
                         if (rawInputFrame.m_buttonSignature != INPUT_mouseScrollVertical)
                                 std::cout << "buttonFlag:\t" << transitionStateToString[rawInputFrame.m_transitionState]
                                           << std::endl;
                         else
                                 std::cout << "mouseDelta:\t" << ((short)rawInputFrame.m_transitionState) << std::endl;*/
                        if (!try_push(rawInputFrame))
                        {
                                while (m_bufferSizes[m_readSwapBufferIndex] != 0)
                                {
                                        Sleep(0);
                                }
                                // TODO // Run a job while we wait?

                                //
                                swap();
                                try_push(rawInputFrame);
                                return;
                        };
                        if (m_bufferSizes[m_readSwapBufferIndex] == 0)
                                swap();

                        Sleep(0);
                }

                void initialize()
                {
                        memset(&m_prevPressState, 0, sizeof(m_prevPressState));
                        m_inputFramesM256 = (volatile __m256i*)_aligned_malloc(m_SZ_M256 * sizeof(__m256i) * m_NUM_SWAP_BUFFERS,
                                                                               alignof(__m256i));

                        m_inputFrames = reinterpret_cast<volatile InputFrameE*>(m_inputFramesM256);

                        reset();
                }

                void shutdown()
                {
                        _aligned_free(const_cast<__m256i*>(m_inputFramesM256));
                }

                void reset()
                {
                        auto inputFrames     = const_cast<InputFrameE*>(m_inputFrames);
                        auto inputFrames256i = reinterpret_cast<__m256i*>(inputFrames);

                        for (auto i = 0; i < m_SZ_M256 * m_NUM_SWAP_BUFFERS; i++)
                                inputFrames256i[i] = _mm256_setzero_si256();
                }

                void invalidate()
                {
                        auto inputFrames     = const_cast<InputFrameE*>(m_inputFrames);
                        auto inputFrames256i = reinterpret_cast<__m256i*>(inputFrames);

                        for (auto i = 0; i < m_SZ_M256; i++)
                                inputFrames256i[i] = _mm256_set1_epi32(-1);
                }
        };

        inline namespace Globals
        {
                inline RawInputBufferE g_inputBufferE;
        }
} // namespace RawInputE