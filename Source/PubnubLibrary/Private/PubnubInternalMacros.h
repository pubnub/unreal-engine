// Copyright 2025 PubNub Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


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
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub is not initialized. Aborting operation. Ensure InitPubnub is called or InitializeAutomatically is enabled."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("Pubnub subsystem is not initialized."), ##__VA_ARGS__); \
			return; \
		} \
		if (!DefaultClient) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: DefaultClient is invalid. Internal systems were not initialized correctly. Try reinitializing Pubnub or contact support."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("PubnubClient is not initialized."), ##__VA_ARGS__); \
			return; \
		} \
		if (!PubnubCallsThread) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub is not initialized. Aborting operation. Ensure InitPubnub is called or InitializeAutomatically is enabled."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			return __VA_ARGS__; \
		} \
		if (!DefaultClient) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: DefaultClient is invalid. Internal systems were not initialized correctly. Try reinitializing Pubnub or contact support."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			return __VA_ARGS__; \
		} \
		if (!PubnubCallsThread) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: %s Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), Message)); \
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
			PubnubError(FString::Printf(TEXT("[%s]: %s Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), Message)); \
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
			PubnubError(FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."); \
			ReturnWrapper.Result = Result; \
			return ReturnWrapper; \
		} \
		if (!PubnubCallsThread) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
			Result.Error = true; \
			Result.ErrorMessage = TEXT("PubnubClient is not initialized. Aborting operation. This client was already destroyed or was not initialized correctly."); \
			return Result; \
		} \
		if (!PubnubCallsThread) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: PubnubCallsThread is invalid. Aborting operation. This client was already destroyed or was not initialized correctly."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)))); \
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
			PubnubError(FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), TEXT(#Field)), EPubnubErrorType::PET_Warning); \
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
			PubnubError(FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), TEXT(#Field)), EPubnubErrorType::PET_Warning); \
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
			PubnubError(FString::Printf(TEXT("[%s]: Another Pubnub operation is in progress. Do not call Sync and Async functions concurrently."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), EPubnubErrorType::PET_Warning); \
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
			PubnubError(FString::Printf(TEXT("[%s]: Another Pubnub operation is in progress. Do not call Sync and Async functions concurrently."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__))), EPubnubErrorType::PET_Warning); \
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
			PubnubError(FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), *UPubnubUtilities::GetNameFromFunctionMacro(ANSI_TO_TCHAR(__FUNCTION__)), TEXT(#Field)), EPubnubErrorType::PET_Warning); \
			return __VA_ARGS__; \
		} \
	} while (false)
