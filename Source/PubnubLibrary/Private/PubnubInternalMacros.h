// Copyright 2024 PubNub Inc. All Rights Reserved.

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
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub is not initialized. Aborting operation. Ensure InitPubnub is called or InitializeAutomatically is enabled."), ANSI_TO_TCHAR(__FUNCTION__))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("Pubnub subsystem is not initialized."), ##__VA_ARGS__); \
			return; \
		} \
		if (!QuickActionThread) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: QuickActionThread is null. Internal systems were not initialized correctly. Try reinitializing Pubnub or contact support."), ANSI_TO_TCHAR(__FUNCTION__))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("QuickActionThread is invalid."), ##__VA_ARGS__); \
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
#define PUBNUB_RETURN_IF_NOT_INITIALIZED() \
	do { \
		if (!IsInitialized) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub is not initialized. Aborting operation. Ensure InitPubnub is called or InitializeAutomatically is enabled."), ANSI_TO_TCHAR(__FUNCTION__))); \
			return; \
		} \
		if (!QuickActionThread) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: QuickActionThread is null. Internal systems were not initialized correctly. Try reinitializing Pubnub or contact support."), ANSI_TO_TCHAR(__FUNCTION__))); \
			return; \
		} \
	} while (false)

/**
 * Verifies that a valid Pubnub user ID has been set before continuing.
 *
 * If the user ID is not set, this macro will:
 *   - Log an error message to the output log
 *   - Invoke the provided delegate with a failure result and optional additional arguments
 *   - Immediately return from the calling function
 *
 */
#define PUBNUB_ENSURE_USER_ID_IS_SET(Delegate, ...) \
	do { \
		if (!IsUserIDSet) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), ANSI_TO_TCHAR(__FUNCTION__))); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, TEXT("Pubnub user ID is not set."), ##__VA_ARGS__); \
			return; \
		} \
	} while (false)

/**
 * Verifies that a valid Pubnub user ID has been set before continuing.
 *
 * If the user ID is not set, this macro will:
 *   - Log an error message to the output log
 *   - Immediately return from the calling function
 *
 */
#define PUBNUB_RETURN_IF_USER_ID_NOT_SET(...) \
	do { \
		if (!IsUserIDSet) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: Pubnub user ID is not set. Aborting operation."), ANSI_TO_TCHAR(__FUNCTION__))); \
			return __VA_ARGS__; \
	} \
} while (false)

/**
 * Validates that the provided field (e.g., channel name, message, metadata) is not empty.
 *
 * If the field is empty, this macro will:
 *   - Log a warning message indicating the missing field
 *   - Call the specified delegate with a failure result including the field name in the error message
 *   - Immediately return from the calling function
 *
 * Usage: Use to guard against invalid or missing user input before executing core logic.
 *
 * @param Field   The field to validate (must implement IsEmpty())
 * @param Delegate The delegate to call on failure
 * @param ...     Additional arguments to pass to the delegate (after the result)
 */
#define PUBNUB_ENSURE_FIELD_NOT_EMPTY(Field, Delegate, ...) \
	do { \
		if (Field.IsEmpty()) \
		{ \
			PubnubError(FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), ANSI_TO_TCHAR(__FUNCTION__), TEXT(#Field)), EPubnubErrorType::PET_Warning); \
			UPubnubUtilities::CallPubnubDelegateWithInvalidArgumentResult(Delegate, FString::Printf(TEXT("Missing required input: '%s' (field is empty). Operation aborted."), TEXT(#Field)), ##__VA_ARGS__); \
		return; \
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
			PubnubError(FString::Printf(TEXT("[%s]: %s field can't be empty. Aborting operation."), ANSI_TO_TCHAR(__FUNCTION__), TEXT(#Field)), EPubnubErrorType::PET_Warning); \
			return __VA_ARGS__; \
		} \
	} while (false)