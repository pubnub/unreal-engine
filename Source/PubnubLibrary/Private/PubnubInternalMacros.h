// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionLibraries/PubnubLogUtilities.h"

/**
 * Formats a single named value as "Name=Value" using PubnubLogUtilities::LogToString.
 *
 * Example:
 *   PUBNUB_LOG_VALUE(Channel)
 */
#define PUBNUB_LOG_VALUE(Var) \
	FString::Printf(TEXT("%s = %s"), TEXT(#Var), *UPubnubLogUtilities::LogToString(Var))

/**
 * Backward-compatible alias for input logging.
 */
#define PUBNUB_LOG_INPUT(Var) \
	PUBNUB_LOG_VALUE(Var)

/**
 * Logs a message prefixed with current function name at the provided level.
 *
 * Usage:
 *   PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Info, TEXT("operation started."));
 */
#define PUBNUB_LOG_FUNCTION(Level, MessageText) \
	do { \
		if (LoggerManager) \
		{ \
			const FString FunctionName = UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)); \
			LoggerManager->Log( \
				Level, \
				EPubnubLogSource::PLS_UE, \
				FString::Printf(TEXT("%s %s"), *FunctionName, *FString(MessageText)), \
				ANSI_TO_TCHAR(__FUNCTION__) \
			); \
		} \
	} while (false)

/**
 * Logs function inputs at Debug level through LoggerManager->Log.
 *
 * Usage:
 *   PUBNUB_LOG_FUNCTION_INPUTS_DEBUG( \
 *       PUBNUB_LOG_INPUT(Channel), \
 *       PUBNUB_LOG_INPUT(Message), \
 *       PUBNUB_LOG_INPUT(PublishSettings) \
 *   );
 */
#define PUBNUB_LOG_FUNCTION_INPUTS_DEBUG(...) \
	do { \
		if (LoggerManager) \
		{ \
			const TArray<FString> InputPairs = { __VA_ARGS__ }; \
			const FString InputsText = FString::Join(InputPairs, TEXT("\n\t-")); \
			const FString FunctionName = UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)); \
			LoggerManager->Log( \
				EPubnubLogLevel::PLL_Debug, \
				EPubnubLogSource::PLS_UE, \
				FString::Printf(TEXT("%s with inputs:\n\t-%s"), *FunctionName, *InputsText), \
				ANSI_TO_TCHAR(__FUNCTION__) \
			); \
		} \
	} while (false)

/**
 * Logs custom debug text and one or more named values.
 *
 * Usage:
 *   PUBNUB_LOG_FUNCTION_DEBUG( \
 *       TEXT("publish completed."), \
 *       PUBNUB_LOG_VALUE(PublishResult) \
 *   );
 */
#define PUBNUB_LOG_FUNCTION_DEBUG(Comment, ...) \
	do { \
		if (LoggerManager) \
		{ \
			const TArray<FString> ValuePairs = { __VA_ARGS__ }; \
			const FString ValuesText = FString::Join(ValuePairs, TEXT("\n\t-")); \
			const FString FunctionName = UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)); \
			const FString ValuesSuffix = ValuesText.IsEmpty() ? TEXT("") : FString::Printf(TEXT("\n\t-%s"), *ValuesText); \
			LoggerManager->Log( \
				EPubnubLogLevel::PLL_Debug, \
				EPubnubLogSource::PLS_UE, \
				FString::Printf(TEXT("%s %s%s"), *FunctionName, *FString(Comment), *ValuesSuffix), \
				ANSI_TO_TCHAR(__FUNCTION__) \
			); \
		} \
	} while (false)

/**
 * Logs custom text at Debug level for current function.
 */
#define PUBNUB_LOG_FUNCTION_DEBUG_TEXT(MessageText) \
	PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Debug, MessageText)

/**
 * Logs custom text at Info level for current function.
 * Use for important lifecycle/operational events (e.g. client ready, reconnect completed).
 */
#define PUBNUB_LOG_FUNCTION_INFO(MessageText) \
	PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Info, MessageText)

/**
 * Logs custom text at Warning level for current function.
 */
#define PUBNUB_LOG_FUNCTION_WARNING(MessageText) \
	PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Warning, MessageText)

/**
 * Logs custom text at Error level for current function.
 */
#define PUBNUB_LOG_FUNCTION_ERROR(MessageText) \
	PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Error, MessageText)

/**
 * Logs an operation result with automatic severity:
 * - Debug when ResultVar.Error is false
 * - Error when ResultVar.Error is true
 *
 * Usage:
 *   PUBNUB_LOG_OPERATION_RESULT(PublishResult);
 */
#define PUBNUB_LOG_OPERATION_RESULT(ResultVar) \
	do { \
		const FPubnubOperationResult& PubnubOpResult = (ResultVar); \
		if (PubnubOpResult.Error) \
		{ \
			PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Error, FString::Printf(TEXT("failed with result:\n\t-%s"), *PUBNUB_LOG_VALUE(ResultVar))); \
		} \
		else \
		{ \
			PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Debug, FString::Printf(TEXT("succeeded with result:\n\t-%s"), *PUBNUB_LOG_VALUE(ResultVar))); \
		} \
	} while (false)

/**
 * Logs a trace message that the current function was called.
 *
 * Usage:
 *   PUBNUB_LOG_FUNCTION_CALLED_TRACE();
 */
#define PUBNUB_LOG_FUNCTION_CALLED_TRACE() \
	PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Trace, TEXT("called."))

/**
 * Logs a trace message with additional custom text for the current function.
 *
 * Usage:
 *   PUBNUB_LOG_FUNCTION_TRACE(FString::Printf(TEXT("State changed: %s"), *State));
 */
#define PUBNUB_LOG_FUNCTION_TRACE(AdditionalText) \
	PUBNUB_LOG_FUNCTION(EPubnubLogLevel::PLL_Trace, AdditionalText)


/**
 * Ensures that the Pubnub subsystem is properly initialized before proceeding.
 *
 * If the subsystem is not initialized or the internal QuickActionThread is invalid,
 * this macro will:
 *   - Log an error message to the output log
 *   - Invoke the provided delegate with a failure result and optional additional arguments
 *   - Immediately return from the calling function (terminating further execution)
 *
 * Usage: Place this at the beginning of any public-facing API to guard against uninitialized use.
 */
#define PUBNUB_ENSURE_INITIALIZED(Delegate, ...) \
	do { \
		if (!IsInitialized) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: Pubnub is not initialized. Aborting operation. Ensure InitPubnub is called or InitializeAutomatically is enabled."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("Pubnub subsystem is not initialized."), ##__VA_ARGS__); \
			return; \
		} \
		if (!DefaultClient) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: DefaultClient is invalid. Internal systems were not initialized correctly. Try reinitializing Pubnub or contact support."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("DefaultClient is invalid."), ##__VA_ARGS__); \
			return; \
		} \
	} while (false)


/**
 * Ensures that the PubnubClient  is properly initialized before proceeding.
 *
 * If the client is not initialized or the internal PubnubCallsThread is invalid,
 * this macro will:
 *   - Log an error message to the output log
 *   - Invoke the provided delegate with a failure result and optional additional arguments
 *   - Immediately return from the calling function (terminating further execution)
 *
 * Usage: Place this at the beginning of any public-facing API to guard against uninitialized use.
 */
#define PUBNUB_ENSURE_CLIENT_INITIALIZED(Delegate, ...) \
	do { \
		if (!IsInitialized) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("PubnubClient is not initialized."), ##__VA_ARGS__); \
			return; \
		} \
		if (!PubnubCallsThread) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("PubnubCallsThread is invalid."), ##__VA_ARGS__); \
			return; \
		} \
	} while (false)

/**
 * Ensures that the Pubnub subsystem is properly initialized before proceeding.
 *
 * If the subsystem is not initialized or the internal QuickActionThread is invalid,
 * this macro will:
 *   - Log an error message to the output log
 *   - Immediately return from the calling function (terminating further execution)
 *
 * Usage: Place this at the beginning of any public-facing API to guard against uninitialized use.
 */
#define PUBNUB_RETURN_IF_NOT_INITIALIZED(...) \
	do { \
		if (!IsInitialized) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: Pubnub is not initialized. Aborting operation. Ensure InitPubnub is called or InitializeAutomatically is enabled."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			return __VA_ARGS__; \
		} \
		if (!DefaultClient) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: DefaultClient is invalid. Internal systems were not initialized correctly. Try reinitializing Pubnub or contact support."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			return __VA_ARGS__; \
		} \
	} while (false)

/**
 * Ensures that the PubnubClient is properly initialized before proceeding.
 *
 * If the subsystem is not initialized or the internal QuickActionThread is invalid,
 * this macro will:
 *   - Log an error message to the output log
 *   - Immediately return from the calling function (terminating further execution)
 *
 * Usage: Place this at the beginning of any public-facing API to guard against uninitialized use.
 */
#define PUBNUB_RETURN_IF_CLIENT_NOT_INITIALIZED(...) \
	do { \
		if (!IsInitialized) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			return __VA_ARGS__; \
		} \
		if (!PubnubCallsThread) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			return __VA_ARGS__; \
		} \
	} while (false)

/**
 * Verifies that a valid Pubnub user ID has been set before continuing.
 *
 * If the user ID is not set, this macro will:
 *   - Log an error message to the output log
 *   - Invoke the provided delegate with a failure result and optional additional arguments
 *   - Immediately return from the calling function
 */
#define PUBNUB_RETURN_IF_USER_ID_NOT_SET(...) \
	do { \
		if (!IsUserIDSet) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Error, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			return __VA_ARGS__; \
		} \
	} while (false)

/**
 * Verifies that the provided condition is met before continuing.
 *
 * If the condition fails, this macro will:
 *   - Log an error message with the provided custom message
 *   - Set the error flag in the provided wrapper struct
 *   - Return the wrapper struct with error information
 *
 * Usage: Use in _priv functions that return wrapper structs for custom validation logic.
 *
 * @param Condition The condition to check (must evaluate to bool)
 * @param Message The error message to display if condition fails
 * @param ReturnWrapper The wrapper struct type to return on failure
 */
#define PUBNUB_RETURN_WRAPPER_IF_CONDITION_FAILS(Condition, Message, ReturnWrapper) \
	do { \
		if (!Condition) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Error, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: %s Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), Message), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = Message; \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
		} \
	} while (false)

/**
 * Verifies that the provided condition is met before continuing.
 *
 * If the condition fails, this macro will:
 *   - Log an error message with the provided custom message
 *   - Return an FPubnubOperationResult with error information
 *
 * Usage: Use in _priv functions that return FPubnubOperationResult for custom validation logic.
 *
 * @param Condition The condition to check (must evaluate to bool)
 * @param Message The error message to display if condition fails
 */
#define PUBNUB_RETURN_OPERATION_RESULT_IF_CONDITION_FAILS(Condition, Message) \
	do { \
		if (!Condition) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Error, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: %s Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), Message), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = Message; \
			return Result; \
		} \
	} while (false)


/**
 * Ensures that the PubnubClient is properly initialized before proceeding.
 *
 * If the client is not initialized or the internal PubnubCallsThread is invalid,
 * this macro will:
 *   - Log an error message to the output log
 *   - Set the error flag in the provided wrapper struct
 *   - Return the wrapper struct with error information
 *
 * Usage: Use in _priv functions that return wrapper structs (e.g., FPubnubPublishMessageResult).
 *
 * @param ReturnWrapper The wrapper struct type to return on failure
 */
#define PUBNUB_RETURN_WRAPPER_IF_NOT_INITIALIZED(ReturnWrapper) \
	do { \
		FPubnubOperationResult Result; \
		if (!IsInitialized) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."); \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
		} \
		if (!PubnubCallsThread) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."); \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
		} \
	} while (false)

/**
 * Ensures that the PubnubClient is properly initialized before proceeding.
 *
 * If the client is not initialized or the internal PubnubCallsThread is invalid,
 * this macro will:
 *   - Log an error message to the output log
 *   - Return an FPubnubOperationResult with error information
 *
 * Usage: Use in _priv functions that return FPubnubOperationResult directly.
 */
#define PUBNUB_RETURN_OPERATION_RESULT_IF_NOT_INITIALIZED() \
	do { \
		FPubnubOperationResult Result; \
		if (!IsInitialized) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."); \
			return Result; \
		} \
		if (!PubnubCallsThread) \
		{ \
			UE_LOG(PubnubLog, Error, TEXT("%s"), *FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."); \
			return Result; \
		} \
	} while (false)


/**
 * Verifies that a valid Pubnub user ID has been set before continuing.
 *
 * If the user ID is not set, this macro will:
 *   - Log an error message to the output log
 *   - Set the error flag in the provided wrapper struct
 *   - Return the wrapper struct with error information
 *
 * Usage: Use in _priv functions that return wrapper structs and require a user ID.
 *
 * @param ReturnWrapper The wrapper struct type to return on failure
 */
#define PUBNUB_RETURN_WRAPPER_IF_USER_ID_NOT_SET(ReturnWrapper) \
	do { \
		if (!IsUserIDSet) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Error, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("Pubnub user ID is not set. Operation aborted."); \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
	} \
	} while (false)

/**
 * Verifies that a valid Pubnub user ID has been set before continuing.
 *
 * If the user ID is not set, this macro will:
 *   - Log an error message to the output log
 *   - Return an FPubnubOperationResult with error information
 *
 * Usage: Use in _priv functions that return FPubnubOperationResult and require a user ID.
 */
#define PUBNUB_RETURN_OPERATION_RESULT_IF_USER_ID_NOT_SET() \
	do { \
		if (!IsUserIDSet) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Error, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("Pubnub user ID is not set. Operation aborted."); \
			return Result; \
		} \
	} while (false)

/**
 * Validates that the provided field (e.g., channel name, message, metadata) is not empty.
 *
 * If the field is empty, this macro will:
 *   - Log a warning message indicating the missing field
 *   - Set the error flag in the provided wrapper struct
 *   - Return the wrapper struct with error information including the field name
 *
 * Usage: Use in _priv functions that return wrapper structs to validate required string inputs.
 *
 * @param Field The field to validate (must implement IsEmpty())
 * @param ReturnWrapper The wrapper struct type to return on failure
 */
#define PUBNUB_RETURN_WRAPPER_IF_FIELD_EMPTY(Field, ReturnWrapper) \
	do { \
		if (Field.IsEmpty()) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Warning, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), TEXT(#Field)), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = FString::Printf(TEXT("Missing required input: '%s' (field is empty). Operation aborted."), TEXT(#Field)); \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
		} \
	} while (false)

/**
 * Validates that the provided field (e.g., channel name, message, metadata) is not empty.
 *
 * If the field is empty, this macro will:
 *   - Log a warning message indicating the missing field
 *   - Return an FPubnubOperationResult with error information including the field name
 *
 * Usage: Use in _priv functions that return FPubnubOperationResult to validate required string inputs.
 *
 * @param Field The field to validate (must implement IsEmpty())
 */
#define PUBNUB_RETURN_OPERATION_RESULT_IF_FIELD_EMPTY(Field) \
	do { \
		if (Field.IsEmpty()) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Warning, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), TEXT(#Field)), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = FString::Printf(TEXT("Missing required input: '%s' (field is empty). Operation aborted."), TEXT(#Field)); \
			return Result; \
		} \
	} while (false)

/**
 * Attempts to acquire the PubnubOperationMutex lock to prevent concurrent operations.
 *
 * If the lock is already held (another operation is in progress), this macro will:
 *   - Log a warning message about concurrent usage
 *   - Set the error flag in the provided wrapper struct
 *   - Return the wrapper struct with error information
 *
 * Usage: Use at the beginning of _priv functions that return wrapper structs to ensure
 *        operations are not called concurrently (mixing Sync and Async is not supported).
 *
 * @param ReturnWrapper The wrapper struct type to return on failure
 */
#define PUBNUB_TRY_LOCK_MUTEX_RETURN_WRAPPER_IF_LOCKED(ReturnWrapper) \
	do { \
		if (!PubnubOperationMutex.TryLock()) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Warning, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: Another Pubnub operation is in progress. Do not call Sync and Async functions concurrently."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = FString::Printf(TEXT("Another Pubnub operation is in progress. Do not call Sync and Async functions concurrently.")); \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
		} \
	} while (false)

/**
 * Attempts to acquire the PubnubOperationMutex lock to prevent concurrent operations.
 *
 * If the lock is already held (another operation is in progress), this macro will:
 *   - Log a warning message about concurrent usage
 *   - Return an FPubnubOperationResult with error information
 *
 * Usage: Use at the beginning of _priv functions that return FPubnubOperationResult to ensure
 *        operations are not called concurrently (mixing Sync and Async is not supported).
 */
#define PUBNUB_TRY_LOCK_MUTEX_RETURN_OPERATION_RESULT_IF_LOCKED() \
	do { \
		if (!PubnubOperationMutex.TryLock()) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Warning, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: Another Pubnub operation is in progress. Do not call Sync and Async functions concurrently."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			FPubnubOperationResult Result; \
			Result.Error = true; \
			Result.ErrorMessage = FString::Printf(TEXT("Another Pubnub operation is in progress. Do not call Sync and Async functions concurrently.")); \
			return Result; \
		} \
	} while (false)


/**
 * Validates that the provided field (e.g., channel name, message, metadata) is not empty.
 *
 * If the field is empty, this macro will:
 *   - Log a warning message indicating the missing field
 *   - Immediately return from the calling function
 *
 * Usage: Use to guard against invalid or missing user input before executing core logic.
 *
 * @param Field   The field to validate (must implement IsEmpty())
 * @param ...     Additional arguments to pass to return statement
 */
#define PUBNUB_RETURN_IF_FIELD_EMPTY(Field, ...) \
	do { \
		if (Field.IsEmpty()) \
		{ \
			if (LoggerManager) \
			{ \
				LoggerManager->Log(EPubnubLogLevel::PLL_Warning, EPubnubLogSource::PLS_UE, FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), TEXT(#Field)), ANSI_TO_TCHAR(__FUNCTION__)); \
			} \
			return __VA_ARGS__; \
		} \
	} while (false)
