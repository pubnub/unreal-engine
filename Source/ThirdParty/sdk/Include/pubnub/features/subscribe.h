/* Copyright (c) PubNub Inc. */
/* See LICENSE in the root directory of this source tree. */

/**
 * @file features/subscribe.h
 * @brief Public API for the subscribe feature (event-engine driven).
 *
 * Subscribe is a long-lived streaming operation driven by an internal
 * event engine. Unlike request/response features (publish, history),
 * subscribe does not return a single future per operation. Instead:
 *
 *   1. Create a subscription handle for each channel/group.
 *   2. Register typed listeners for event dispatch.
 *   3. Start the subscription — the SDK maintains the long-poll loop.
 *      On threaded builds (PUBNUB_CFG_THREAD_SAFETY=1), a background
 *      thread starts automatically to drive I/O and deliver listener
 *      callbacks. On cooperative builds, call `pubnub_process(ctx)` in
 *      your event loop.
 *
 * ## Usage (cooperative / callback)
 *
 * @code
 * void on_message(const pubnub_subscribe_event_t* event, void* user_data) {
 *     pubnub_context_t*                ctx = (pubnub_context_t*)user_data;
 *     pubnub_subscribe_message_event_t msg;
 *     if (PUBNUB_OK != pubnub_subscribe_event_message(ctx, event, &msg)) {
 *         return;
 *     }
 *     pubnub_serialization_provider_t* serial = pubnub_serialization(ctx);
 *     size_t      len = 0;
 *     const char* p   = serial->value_as_string(msg.message, &len);
 *     printf("msg: %.*s\n", (int)len, p ? p : "");
 * }
 *
 * pubnub_subscribe_listener_t listener = {
 *     .on_message = on_message, .user_data = ctx,
 * };
 * pubnub_add_listener(ctx, &listener);
 *
 * pubnub_entity_t entity = pubnub_channel(ctx, "my-channel");
 * pubnub_subscription_t sub = pubnub_subscription_create(
 *     entity, &(pubnub_subscription_opts_t){.with_presence = 1});
 * pubnub_entity_destroy(entity);
 * pubnub_subscription_subscribe(sub);
 *
 * // Event loop — callbacks fire inside pubnub_process():
 * while (running) {
 *     pubnub_process(ctx);
 * }
 *
 * pubnub_subscription_unsubscribe(sub);
 * pubnub_subscription_destroy(sub);
 * @endcode
 *
 * ## Required context configuration
 *
 *   - `pubnub_config_t::subscribe_key` MUST be set.
 *   - `pubnub_config_t::user_id`       MUST be set.
 */

#ifndef PUBNUB_FEATURE_SUBSCRIBE_H
#define PUBNUB_FEATURE_SUBSCRIBE_H

#include "pubnub/config.h"

#if PUBNUB_ENABLE_SUBSCRIBE

#include "pubnub/error.h"
#include "pubnub/features/subscribe_types.h"
#include "pubnub/future.h"
#include "pubnub/types_fwd.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
// clang-format off
extern "C" {
// clang-format on
#endif

/**
 * @brief Create a channel entity handle.
 *
 * @param ctx  Initialized context (@b required, @b borrowed). Must have
 *             the subscribe feature enabled.
 * @param name NUL-terminated channel name (@b required, copied by the
 *             SDK). The PubNub server enforces a 2048-character limit.
 * @return Entity handle on success, or @c NULL on validation/allocation
 *         failure.
 */
PUBNUB_API pubnub_entity_t pubnub_channel(pubnub_context_t* ctx, const char* name);

/**
 * @brief Create a channel group entity handle.
 *
 * @param ctx  Initialized context (@b required, @b borrowed).
 * @param name NUL-terminated channel group name (@b required, copied by
 *             the SDK). The PubNub server enforces a 92-character limit.
 * @return Entity handle on success, or @c NULL on failure.
 */
PUBNUB_API pubnub_entity_t pubnub_channel_group(pubnub_context_t* ctx,
                                                const char*       name);

/**
 * @brief Create a channel metadata entity handle (App Context).
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @param id  NUL-terminated metadata object identifier (@b required,
 *            copied by the SDK).
 * @return Entity handle on success, or @c NULL on failure.
 */
PUBNUB_API pubnub_entity_t pubnub_channel_metadata(pubnub_context_t* ctx,
                                                   const char*       id);

/**
 * @brief Create a user metadata entity handle (App Context).
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @param id  NUL-terminated metadata object identifier (@b required,
 *            copied by the SDK).
 * @return Entity handle on success, or @c NULL on failure.
 */
PUBNUB_API pubnub_entity_t pubnub_user_metadata(pubnub_context_t* ctx,
                                                const char*       id);

/**
 * @brief Destroy an entity handle and release its registry entry.
 *
 * @attention Do not use `entity` after this call.
 *
 * @param entity Entity handle to destroy (@b consumed). Do not use after
 *               this call.
 */
PUBNUB_API void pubnub_entity_destroy(pubnub_entity_t entity);

/**
 * @brief Get the name (or identifier) of an entity.
 *
 * Returns a pointer to the NUL-terminated string which is valid
 * as long as the entity exists (i.e., at least one entity handle
 * or subscription still references it).
 *
 * @param entity Entity handle (@b required).
 * @return Borrowed pointer to the entity name, or @c NULL if the
 *         handle is @c NULL or the entry has been released.
 */
PUBNUB_API const char* pubnub_entity_name(pubnub_entity_t entity);

/**
 * @brief Get the type of entity.
 *
 * @param entity Entity handle (@b required).
 * @return Entity type, or @c PUBNUB_SUBSCRIBE_CHANNEL if handle is @c NULL.
 */
PUBNUB_API pubnub_subscribe_entity_type_t pubnub_entity_type(pubnub_entity_t entity);

/**
 * @brief Create a subscription from an entity handle.
 *
 * @param entity Entity handle (@b required, @b borrowed).
 * @param opts   Subscription options (@b optional, @b borrowed). Pass @c NULL
 *               for defaults. @c with_presence is silently ignored for
 *               metadata entities.
 * @return Subscription handle on success. @c NULL on validation failure
 *         (NULL entity) or allocation failure (out of memory).
 *
 * @note The entity handle may be destroyed after creating the subscription.
 */
PUBNUB_API pubnub_subscription_t
pubnub_subscription_create(pubnub_entity_t                   entity,
                           const pubnub_subscription_opts_t* opts);

/**
 * @brief Destroy a subscription and release associated resources.
 *
 * @attention Do not use `entity` after this call.
 *
 * @param sub Subscription handle to destroy (@b consumed). Do not use
 *            after this call.
 */
PUBNUB_API void pubnub_subscription_destroy(pubnub_subscription_t sub);

/**
 * @brief Activate a subscription (begin receiving events).
 *
 * @param sub Subscription handle (@b required).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if sub is NULL;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe feature is not
 *         registered.
 */
PUBNUB_API pubnub_res_t pubnub_subscription_subscribe(pubnub_subscription_t sub);

/**
 * @brief Deactivate a subscription (stop receiving on this channel).
 *
 * @param sub Subscription handle (@b required).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if sub is @c NULL.
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe state is inconsistent
 *         (subscription marked active but manager absent; should not happen
 *         under normal usage).
 */
PUBNUB_API pubnub_res_t pubnub_subscription_unsubscribe(pubnub_subscription_t sub);

/**
 * @brief Register a context-global listener for subscribe events.
 *
 * Receives events from ALL active subscriptions on the context.
 *
 * @param ctx      Initialized context (@b required, @b borrowed).
 * @param listener Listener struct with typed callbacks (@b required,
 *                 @b borrowed — contents are copied into the SDK).
 *                 @c NULL callbacks are skipped during dispatch.
 * @return Listener handle on success, or
 *         @c PUBNUB_LISTENER_HANDLE_INVALID when the listener
 *         registry is full or arguments are invalid.
 *
 * @note This is the only listener level that receives status events
 * (`connected`, `disconnected`, etc.) — per-subscription and per-set
 * listeners receive data events only.
 */
PUBNUB_API pubnub_listener_handle_t
pubnub_add_listener(pubnub_context_t*                  ctx,
                    const pubnub_subscribe_listener_t* listener);

/**
 * @brief Remove a context-global listener.
 *
 * @note If you free @c user_data immediately after this call AND
 *       a callback may be in progress on the background thread, call
 *       this function from within the callback itself or after
 *       disconnect + drain to guarantee no concurrent access.
 *
 * @param ctx    Initialized context (@b required, @b borrowed).
 * @param handle Handle returned by @c pubnub_add_listener.
 */
PUBNUB_API void pubnub_remove_listener(pubnub_context_t*        ctx,
                                       pubnub_listener_handle_t handle);

/**
 * @brief Register a listener bound to a specific subscription.
 *
 * Receives data events only from the channel/group associated with
 * @p sub.
 *
 * @param sub      Subscription handle (@b required, @b borrowed).
 * @param listener Listener struct with typed callbacks (@b required,
 *                 @b borrowed — contents are copied into the SDK).
 *                 @c NULL callbacks are skipped during dispatch.
 * @return Listener handle on success, or
 *         @c PUBNUB_LISTENER_HANDLE_INVALID on failure.
 */
PUBNUB_API pubnub_listener_handle_t
pubnub_subscription_add_listener(pubnub_subscription_t              sub,
                                 const pubnub_subscribe_listener_t* listener);

/**
 * @brief Remove a per-subscription listener.
 *
 * @note If you free @c user_data immediately after this call AND
 *       a callback may be in progress on the background thread, call
 *       this function from within the callback itself or after
 *       disconnect + drain to guarantee no concurrent access.
 *
 * @param sub    Subscription handle (@b required, @b borrowed).
 * @param handle Handle returned by @c pubnub_subscription_add_listener.
 */
PUBNUB_API void pubnub_subscription_remove_listener(pubnub_subscription_t sub,
                                                    pubnub_listener_handle_t handle);

/**
 * @brief Create an empty subscription set.
 *
 * A subscription set groups multiple subscriptions that are activated
 * or deactivated atomically. Subscriptions are added to the set
 * via @c pubnub_subscription_set_add_subscription.
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @return Set handle on success, or
 *         @c PUBNUB_SUBSCRIPTION_SET_INVALID on failure.
 */
PUBNUB_API pubnub_subscription_set_t
pubnub_subscription_set_create(pubnub_context_t* ctx);

/**
 * @brief Add a subscription to a subscription set.
 *
 * The same subscription can be in a set AND subscribed individually.
 * The active_count on the registry entry tracks all references.
 * Presence leave is only sent when the last reference deactivates.
 *
 * If the subscription's entry is already present in the set, this is
 * a no-op returning @c PUBNUB_OK (deduplication by entry_index).
 *
 * If the set is already subscribed (activated), a newly added entry
 * is auto-activated and begins receiving events immediately.
 *
 * @param set  Set handle (@b required, @b borrowed).
 * @param sub  Subscription handle (@b required, @b borrowed). Must belong
 *             to the same context as the set.
 * @retval PUBNUB_OK on success (or already present);
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if set or sub is NULL, or
 *         the subscription belongs to a different context;
 * @retval PUBNUB_ERR_QUEUE_FULL if the set is at capacity.
 */
PUBNUB_API pubnub_res_t
pubnub_subscription_set_add_subscription(pubnub_subscription_set_t set,
                                         pubnub_subscription_t     sub);

/**
 * @brief Merge all entries from another subscription set.
 *
 * For each entry in @p other, if it is not already present in
 * @p target, a new reference is acquired and the entry is appended.
 * Duplicate entries (same entry_index already in target) are skipped.
 *
 * If @p target is already subscribed (activated), newly merged entries
 * are auto-activated and begin receiving events immediately.
 *
 * @param target Destination set handle (@b required, @b borrowed).
 * @param other  Source set handle (@b required, @b borrowed). Must belong
 *               to the same context as @p target. Not consumed.
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if either set is NULL, invalid,
 *         or they belong to different contexts;
 * @retval PUBNUB_ERR_QUEUE_FULL if the target set reaches capacity
 *         during the merge (partial merge may have occurred);
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not registered.
 */
PUBNUB_API pubnub_res_t
pubnub_subscription_set_add_subscription_set(pubnub_subscription_set_t target,
                                             pubnub_subscription_set_t other);

/**
 * @brief Remove a subscription from a subscription set.
 *
 * If the subscription's entry is not a member of the set, returns
 * PUBNUB_ERR_INVALID_ARGUMENT.
 *
 * If the set is currently subscribed, the removed entry's active_count
 * is decremented. If that was the last active reference to the entity
 * (active_count reaches 0), presence leave is triggered and the
 * subscribe loop is updated.
 *
 * @param set Set handle (@b required, @b borrowed).
 * @param sub Subscription handle (@b required, @b borrowed). Must belong
 *            to the same context as the set.
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if set/sub is NULL, different
 *         context, or subscription is not a member of the set;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not registered.
 */
PUBNUB_API pubnub_res_t
pubnub_subscription_set_remove_subscription(pubnub_subscription_set_t set,
                                            pubnub_subscription_t     sub);

/**
 * @brief Remove all entries found in another set from this set.
 *
 * For each entry in @p other that is also present in @p target,
 * the entry is removed from target (ref released, active_count
 * decremented if target is subscribed).
 *
 * If the target set is subscribed and any removed entry's
 * active_count reaches 0, presence leave is triggered.
 *
 * @param target Set to remove from (@b required, @b borrowed).
 * @param other  Set whose entries to remove from target (@b required,
 *               @b borrowed). Must be same context. Not consumed.
 * @retval PUBNUB_OK on success (entries not found are silently skipped);
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if either set is @c NULL/invalid or
 *         different contexts;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not registered.
 */
PUBNUB_API pubnub_res_t
pubnub_subscription_set_remove_subscription_set(pubnub_subscription_set_t target,
                                                pubnub_subscription_set_t other);

/**
 * @brief Activate all subscriptions in a set (begin receiving events).
 *
 * @param set Set handle (@b required, carries context from creation).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if set is invalid;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not registered.
 */
PUBNUB_API pubnub_res_t pubnub_subscription_set_subscribe(pubnub_subscription_set_t set);

/**
 * @brief Deactivate all subscriptions in a set.
 *
 * @param set Set handle (@b required, carries context from creation).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if set is invalid;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not registered.
 */
PUBNUB_API pubnub_res_t pubnub_subscription_set_unsubscribe(pubnub_subscription_set_t set);

/**
 * @brief Destroy a subscription set and release all its entries.
 *
 * If the member was active AND this set held the last active
 * reference to that subscription / entity presence leave is
 * triggered.
 *
 * @attention Do not use `set` after this call.
 *
 * @param set Set handle (@b consumed). Do not use after this call.
 */
PUBNUB_API void pubnub_subscription_set_destroy(pubnub_subscription_set_t set);

/**
 * @brief Register a listener bound to a subscription set.
 *
 * Receives data events only from channels/groups that are members of
 * the specified set.
 *
 * @param set      Set handle (@b required, @b borrowed).
 * @param listener Listener struct with typed callbacks (@b required,
 *                 @b borrowed — contents are copied into the SDK). The
 *                 @c on_status field is ignored for per-set listeners.
 * @retval Listener handle on success, or
 * @retval PUBNUB_LISTENER_HANDLE_INVALID on failure.
 */
PUBNUB_API pubnub_listener_handle_t pubnub_subscription_set_add_listener(
    pubnub_subscription_set_t          set,
    const pubnub_subscribe_listener_t* listener);

/**
 * @brief Remove a per-set listener.
 *
 * @note If you free @c user_data immediately after this call AND
 *       a callback may be in progress on the background thread, call
 *       this function from within the callback itself or after
 *       disconnect + drain to guarantee no concurrent access.
 *
 * @param set    Set handle (@b required, @b borrowed).
 * @param handle Handle returned by
 *               @c pubnub_subscription_set_add_listener.
 */
PUBNUB_API void
pubnub_subscription_set_remove_listener(pubnub_subscription_set_t set,
                                        pubnub_listener_handle_t  handle);

/**
 * @brief Disconnect from all channels and stop the subscribe loop.
 *
 * If the client was actively receiving (CONNECTED state), event listeners
 * receive a @c PUBNUB_SUBSCRIBE_STATUS_DISCONNECTED status event. If called
 * while still connecting (HANDSHAKING state), the in-flight handshake is
 * cancelled and no status event is emitted.
 *
 * Subscriptions remain registered and can be restarted via
 * @c pubnub_subscribe_reconnect.
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if ctx is NULL;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not active.
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_disconnect(pubnub_context_t* ctx);

/**
 * @brief Reconnect after a disconnect or failure.
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if ctx is NULL;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not active.
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_reconnect(pubnub_context_t* ctx);

/**
 * @brief Store a resume cursor and emit a SUBSCRIPTION_RESTORED event.
 *
 * Saves @p timetoken as the resume cursor and signals the internal event
 * engine that the cursor should be used for the next receive cycle. The
 * effect depends on the current subscribe state:
 *
 * - **Connected (RECEIVING/HANDSHAKING):** the next receive cycle starts
 *   from @p timetoken; no re-handshake is issued.
 * - **Stopped or disconnected:** the cursor is saved but no connection
 *   starts. Call @c pubnub_subscribe_reconnect() after this function to
 *   restart from the saved position.
 *
 * Typical use after an app restart:
 * @code
 *   pubnub_subscribe_restore(ctx, saved_timetoken);
 *   pubnub_subscribe_reconnect(ctx);
 * @endcode
 *
 * Passing a zero-length or NULL timetoken only emits SUBSCRIPTION_RESTORED
 * using the internally stored cursor (equivalent to
 * pubnub_subscribe_reconnect() in terms of resume position).
 *
 * @param ctx       Context handle (@b required, non-NULL).
 * @param timetoken Cursor to restore from. @c pubnub_timetoken_t is a
 *                  typedef for @c pubnub_string_view_t; the view is
 *                  consumed by this call and need not remain valid
 *                  afterwards.
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if @p ctx is NULL;
 * @retval PUBNUB_ERR_NOT_INITIALIZED if subscribe is not active.
 *
 * @see pubnub_subscribe_reconnect
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_restore(pubnub_context_t*  ctx,
                                                 pubnub_timetoken_t timetoken);

/**
 * @brief Unsubscribe from all channels and groups.
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if ctx is @c NULL.
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_unsubscribe_all(pubnub_context_t* ctx);

/**
 * @brief Query the current subscribe connection state.
 *
 * @param ctx Initialized context (@b required, @b borrowed).
 * @return Current connection state or @c PUBNUB_SUBSCRIBE_IDLE
 *         when the subscribe feature is not initialized or ctx is @c NULL.
 */
PUBNUB_API pubnub_subscribe_connection_state_t
pubnub_subscribe_state(const pubnub_context_t* ctx);

/**
 * @brief List all active subscriptions on this context.
 *
 * Fills @p out with handles for subscriptions currently in the
 * subscribed state. The returned handles are borrowed -- valid until
 * the corresponding subscription is destroyed. Handles may be passed
 * to @c pubnub_subscription_unsubscribe, @c pubnub_entity_name, etc.
 *
 * Typical usage to check if a subscription is still active:
 * @code
 * pubnub_subscription_t active[PUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS];
 * size_t count = 0;
 * pubnub_subscriptions(ctx, active,
 *                      PUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS, &count);
 * @endcode
 *
 * @param ctx       Initialized context (@b required, @b borrowed).
 * @param out       Caller-allocated array to receive handles
 *                  (@b required).
 * @param max_count Capacity of @p out.
 * @param out_count Receives the total number of matching items, even
 *                  when larger than @p max_count. When
 *                  @c PUBNUB_ERR_BUFFER_TOO_SMALL is returned, use
 *                  @c *out_count to allocate a larger buffer and retry.
 *                  May be @c NULL.
 * @return @c PUBNUB_OK on success, @c PUBNUB_ERR_BUFFER_TOO_SMALL if
 *         @p max_count was insufficient (partial fill of @p out).
 */
PUBNUB_API pubnub_res_t pubnub_subscriptions(pubnub_context_t*      ctx,
                                             pubnub_subscription_t* out,
                                             size_t                 max_count,
                                             size_t*                out_count);

/**
 * @brief List all active subscription sets on this context.
 *
 * @param ctx       Initialized context (@b required, @b borrowed).
 * @param out       Caller-allocated array (@b required).
 * @param max_count Capacity of @p out.
 * @param out_count Receives the total number of matching items, even
 *                  when larger than @p max_count. When
 *                  @c PUBNUB_ERR_BUFFER_TOO_SMALL is returned, use
 *                  @c *out_count to allocate a larger buffer and retry.
 *                  May be @c NULL.
 * @return @c PUBNUB_OK on success, @c PUBNUB_ERR_BUFFER_TOO_SMALL if
 *         truncated.
 */
PUBNUB_API pubnub_res_t pubnub_subscription_sets(pubnub_context_t*          ctx,
                                                 pubnub_subscription_set_t* out,
                                                 size_t  max_count,
                                                 size_t* out_count);

/**
 * @brief List all subscriptions within a subscription set.
 *
 * Returns subscription handles for all entries that are members of
 * the set, regardless of individual subscription state (the set
 * manages activation at the set level).
 *
 * @note Results are matched by underlying channel/group entry
 *       identity, not by the subscription handle passed to
 *       @c pubnub_subscription_set_add_subscription. If multiple
 *       subscription handles reference the same entity, all are
 *       returned.
 *
 * @param set       Subscription set to query (@b required, @b borrowed).
 * @param out       Caller-allocated array (@b required).
 * @param max_count Capacity of @p out.
 * @param out_count Receives the total number of matching items, even
 *                  when larger than @p max_count. When
 *                  @c PUBNUB_ERR_BUFFER_TOO_SMALL is returned, use
 *                  @c *out_count to allocate a larger buffer and retry.
 *                  May be @c NULL.
 * @return @c PUBNUB_OK on success, @c PUBNUB_ERR_BUFFER_TOO_SMALL if
 *         truncated.
 */
PUBNUB_API pubnub_res_t
pubnub_subscription_set_subscriptions(pubnub_subscription_set_t set,
                                      pubnub_subscription_t*    out,
                                      size_t                    max_count,
                                      size_t*                   out_count);

/**
 * @brief Extract a typed message event from a subscribe event.
 *
 * Validates that the event type is @c PUBNUB_SUBSCRIBE_MESSAGE and
 * copies the event into the caller-owned output struct.
 *
 * @param ctx   Initialized context (borrowed).
 * @param event Subscribe event of type `PUBNUB_SUBSCRIBE_MESSAGE`.
 * @param out   Caller-owned struct (populated on success, zero-
 *              initialized on failure).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if any arg is @c NULL or event
 *         type is not MESSAGE;
 * @retval PUBNUB_ERR_SERIALIZATION on parse failure.
 *
 * @code
 * pubnub_subscribe_message_event_t msg;
 * if (PUBNUB_OK == pubnub_subscribe_event_message(ctx, &ev, &msg)) {
 *     pubnub_serialization_provider_t* serial = pubnub_serialization(ctx);
 *     size_t      plen = 0;
 *     const char* p    = serial->value_as_string(msg.message, &plen);
 *     printf("ch=%.*s payload=%.*s\n",
 *            (int)msg.channel.len, msg.channel.ptr,
 *            (int)plen, p ? p : "");
 * }
 * @endcode
 */
PUBNUB_API pubnub_res_t
pubnub_subscribe_event_message(pubnub_context_t*                 ctx,
                               const pubnub_subscribe_event_t*   event,
                               pubnub_subscribe_message_event_t* out);

/**
 * @brief Extract a typed signal event from a subscribe event.
 *
 * Validates that the event type is @c PUBNUB_SUBSCRIBE_SIGNAL and
 * copies the event into the caller-owned output struct.
 *
 * @param ctx   Initialized context (borrowed).
 * @param event Subscribe event of type `PUBNUB_SUBSCRIBE_SIGNAL`.
 * @param out   Caller-owned struct (populated on success, zero-
 *              initialized on failure).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if any arg is @c NULL or event
 *         type is not SIGNAL;
 * @retval PUBNUB_ERR_SERIALIZATION on parse failure.
 *
 * @code
 * pubnub_subscribe_signal_event_t sig;
 * if (PUBNUB_OK == pubnub_subscribe_event_signal(ctx, &ev, &sig)) {
 *     pubnub_serialization_provider_t* serial = pubnub_serialization(ctx);
 *     size_t      plen = 0;
 *     const char* p    = serial->value_as_string(sig.message, &plen);
 *     printf("signal on %.*s: %.*s\n",
 *            (int)sig.channel.len, sig.channel.ptr,
 *            (int)plen, p ? p : "");
 * }
 * @endcode
 */
PUBNUB_API pubnub_res_t
pubnub_subscribe_event_signal(pubnub_context_t*                ctx,
                              const pubnub_subscribe_event_t*  event,
                              pubnub_subscribe_signal_event_t* out);

/**
 * @brief Extract presence details from a subscribe event.
 *
 * Validates that the event type is @c PUBNUB_SUBSCRIBE_PRESENCE and
 * copies the event into the caller-owned output struct.
 *
 * @param ctx   Initialized context (borrowed).
 * @param event Subscribe event of type `PUBNUB_SUBSCRIBE_PRESENCE`.
 * @param out   Caller-owned presence event struct (populated on
 *              success, zero-initialized on failure).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if any arg is @c NULL or event
 *         type is not PRESENCE;
 * @retval PUBNUB_ERR_SERIALIZATION on parse failure.
 *
 * @code
 * pubnub_subscribe_presence_event_t pres;
 * if (PUBNUB_OK == pubnub_subscribe_event_presence(ctx, &ev, &pres)) {
 *     printf("action=%d channel=%.*s occupancy=%u\n",
 *            pres.action, (int)pres.channel.len, pres.channel.ptr,
 *            pres.occupancy);
 *     if (PUBNUB_PRESENCE_INTERVAL == pres.action) {
 *         if (pres.here_now_refresh) {
 *             printf("Full list too large; call hereNow.\n");
 *         } else {
 *             pubnub_serialization_provider_t* serial =
 *                 pubnub_serialization(ctx);
 *             if (NULL != pres.joined) {
 *                 for (size_t k = 0;
 *                      k < serial->array_size(pres.joined); ++k) {
 *                     const pubnub_json_value_t* node =
 *                         serial->array_get(pres.joined, k);
 *                     size_t      ulen = 0;
 *                     const char* u =
 *                         serial->value_as_string(node, &ulen);
 *                     printf("joined: %.*s\n",
 *                            (int)ulen, u ? u : "");
 *                 }
 *             }
 *             if (NULL != pres.left) {
 *                 for (size_t k = 0;
 *                      k < serial->array_size(pres.left); ++k) {
 *                     const pubnub_json_value_t* node =
 *                         serial->array_get(pres.left, k);
 *                     size_t      ulen = 0;
 *                     const char* u =
 *                         serial->value_as_string(node, &ulen);
 *                     printf("left: %.*s\n",
 *                            (int)ulen, u ? u : "");
 *                 }
 *             }
 *             if (NULL != pres.timed_out) {
 *                 for (size_t k = 0;
 *                      k < serial->array_size(pres.timed_out); ++k) {
 *                     const pubnub_json_value_t* node =
 *                         serial->array_get(pres.timed_out, k);
 *                     size_t      ulen = 0;
 *                     const char* u =
 *                         serial->value_as_string(node, &ulen);
 *                     printf("timed out: %.*s\n",
 *                            (int)ulen, u ? u : "");
 *                 }
 *             }
 *         }
 *     } else {
 *         printf("uuid=%.*s\n",
 *                (int)pres.uuid.len, pres.uuid.ptr);
 *     }
 * }
 * @endcode
 */
PUBNUB_API pubnub_res_t
pubnub_subscribe_event_presence(pubnub_context_t*                  ctx,
                                const pubnub_subscribe_event_t*    event,
                                pubnub_subscribe_presence_event_t* out);

/**
 * @brief Extract message action details from a subscribe event.
 *
 * Validates that the event type is @c PUBNUB_SUBSCRIBE_MESSAGE_ACTION and
 * copies the event into the caller-owned output struct.
 *
 * @param ctx   Initialized context (borrowed).
 * @param event Subscribe event of type `PUBNUB_SUBSCRIBE_MESSAGE_ACTION`.
 * @param out   Caller-owned struct (populated on success).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if any arg is @c NULL or event
 *         type is not MESSAGE_ACTION;
 * @retval PUBNUB_ERR_SERIALIZATION on parse failure.
 *
 * @code
 * pubnub_subscribe_message_action_event_t ma;
 * if (PUBNUB_OK ==
 *     pubnub_subscribe_event_message_action(ctx, &ev, &ma)) {
 *     printf("action=%.*s value=%.*s on msg_tt=%.*s\n",
 *            (int)ma.type.len, ma.type.ptr,
 *            (int)ma.value.len, ma.value.ptr,
 *            (int)ma.message_timetoken.len,
 *            ma.message_timetoken.ptr);
 * }
 * @endcode
 */
PUBNUB_API pubnub_res_t pubnub_subscribe_event_message_action(
    pubnub_context_t*                        ctx,
    const pubnub_subscribe_event_t*          event,
    pubnub_subscribe_message_action_event_t* out);

/**
 * @brief Extract App Context details from a subscribe event.
 *
 * Validates that the event type is @c PUBNUB_SUBSCRIBE_APP_CONTEXT
 * and copies the event into the caller-owned output struct.
 *
 * @param ctx   Initialized context (borrowed).
 * @param event Subscribe event of type `PUBNUB_SUBSCRIBE_APP_CONTEXT`.
 * @param out   Caller-owned struct (populated on success).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if any arg is @c NULL or event
 *         type is not APP_CONTEXT;
 * @retval PUBNUB_ERR_SERIALIZATION on parse failure.
 *
 * @code
 * pubnub_subscribe_app_context_event_t obj;
 * if (PUBNUB_OK == pubnub_subscribe_event_app_context(ctx, &ev, &obj)) {
 *     const char* type_str;
 *     switch (obj.object_type) {
 *     case PUBNUB_APP_CONTEXT_OBJECT_UUID:
 *         type_str = "uuid";
 *         break;
 *     case PUBNUB_APP_CONTEXT_OBJECT_CHANNEL:
 *         type_str = "channel";
 *         break;
 *     case PUBNUB_APP_CONTEXT_OBJECT_MEMBERSHIP:
 *         type_str = "membership";
 *         break;
 *     default:
 *         type_str = "unknown";
 *         break;
 *     }
 *     printf("app_context %s event on %.*s\n",
 *            type_str, (int)obj.channel.len, obj.channel.ptr);
 * }
 * @endcode
 */
PUBNUB_API pubnub_res_t
pubnub_subscribe_event_app_context(pubnub_context_t*                     ctx,
                                   const pubnub_subscribe_event_t*       event,
                                   pubnub_subscribe_app_context_event_t* out);

/**
 * @brief Extract file sharing details from a subscribe event.
 *
 * Validates that the event type is @c PUBNUB_SUBSCRIBE_FILE and
 * copies the event into the caller-owned output struct.
 *
 * @param ctx   Initialized context (borrowed).
 * @param event Subscribe event of type `PUBNUB_SUBSCRIBE_FILE`.
 * @param out   Caller-owned struct (populated on success).
 * @retval PUBNUB_OK on success;
 * @retval PUBNUB_ERR_INVALID_ARGUMENT if any arg is @c NULL or event
 *         type is not FILE;
 * @retval PUBNUB_ERR_SERIALIZATION on parse failure.
 *
 * @code
 * pubnub_subscribe_file_event_t file;
 * if (PUBNUB_OK == pubnub_subscribe_event_file(ctx, &ev, &file)) {
 *     printf("file_id=%.*s name=%.*s\n",
 *            (int)file.file_id.len, file.file_id.ptr,
 *            (int)file.file_name.len, file.file_name.ptr);
 * }
 * @endcode
 */
PUBNUB_API pubnub_res_t
pubnub_subscribe_event_file(pubnub_context_t*               ctx,
                            const pubnub_subscribe_event_t* event,
                            pubnub_subscribe_file_event_t*  out);

#ifdef __cplusplus
// clang-format off
}
// clang-format on
#endif

#endif /* PUBNUB_ENABLE_SUBSCRIBE */

#endif /* PUBNUB_FEATURE_SUBSCRIBE_H */
