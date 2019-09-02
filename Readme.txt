ECS Guide:
==========
Components
	-Has: 	state
	-No: 	functionality
Systems
	-No: 	state
	-Has: 	functionality (mutate Components/State)

	-Has:	helpers (private member functions)
	-Has:	utility function calls
Entities
	-Has: 	index state
	-No: 	other state

	-Has: 	helper functions
==========
Helpers (private system member functions)
	-Has: 	private access 
	-No:	public access
Utilities (public headers with public functions that mutate Components/State)
	-Has: 	public access and can be called from different systems