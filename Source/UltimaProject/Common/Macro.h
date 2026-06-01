#pragma once

#define NULLCHECK(variable) if(variable == nullptr) { \
	return; \
}

#define NULLCHECK_RETURN(variable, return_value) if(variable == nullptr) {\
	return return_value; \
}

#define NULLCHECK_RETURN_LOG(variable, return_value, log_message) if(variable == nullptr) {\
	UE_LOG(LogTemp, Log, TEXT(log_message));\
	return return_value; \
}