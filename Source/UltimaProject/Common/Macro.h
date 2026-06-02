#pragma once

#define NULLCHECK(variable) if(variable == nullptr) { \
	return; \
}

#define NULLCHECK_RETURN(variable, return_value) if(variable == nullptr) {\
	return return_value; \
}

#define NULLCHECK_LOG(variable, verbosity, log_message) if(variable == nullptr) {\
	UE_LOG(LogTemp, verbosity, TEXT(log_message));\
	return;\
}

#define NULLCHECK_RETURN_LOG(variable, return_value, verbosity, log_message) if(variable == nullptr) {\
	UE_LOG(LogTemp, verbosity, TEXT(log_message));\
	return return_value; \
}

#define NULLCHECK_SP(variable) if(!variable.IsValid()) { \
	return; \
}

#define NULLCHECK_SP_RETURN(variable, return_value) if(!variable.IsValid()) {\
	return return_value; \
}

#define NULLCHECK_SP_LOG(variable, verbosity, log_message) if(!variable.IsValid()) {\
	UE_LOG(LogTemp, verbosity, TEXT(log_message));\
	return; \
}

#define NULLCHECK_SP_RETURN_LOG(variable, return_value, verbosity, log_message) if(!variable.IsValid()) {\
	UE_LOG(LogTemp, verbosity, TEXT(log_message));\
	return return_value; \
}
