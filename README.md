# PubNub Unreal Engine SDK

<p align="center">
  <img src="https://raw.githubusercontent.com/pubnub/rust/master/logo.svg" alt="PubNub" width="300"/>
</p>

[![License: custom MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/pubnub/rust/LICENSE)

Welcome to the official PubNub Unreal Engine SDK repository. [PubNub](https://www.pubnub.com/) provides the infrastructure and APIs needed for the realtime communication layer of your application. You can focus on building your app's logic while PubNub ensures data is sent and received across the world in less than 100ms.

## Getting Started

Below, you will find everything you need to begin messaging!

### Get PubNub Keys

You will need publish and subscribe keys to authenticate your app. Get your keys from the [Admin Portal](https://dashboard.pubnub.com/login).

### Configure Unreal Engine

1. Download and [install Unreal Engine](https://dev.epicgames.com/documentation/pl-pl/unreal-engine/installing-unreal-engine) version 5.0 or higher.
2. Create a new blank Unreal project at a location of your choice.
3. Create an empty `Plugins` folder in the location of your Unreal project.

### Download the SDK

Within the `Plugins` folder of your Unreal project, clone the Unreal SDK repository: [https://github.com/pubnub/unreal-engine](https://github.com/pubnub/unreal-engine "SDK source-code repository on GitHub").

### Blueprints and Sample Code

Each PubNub API is available through Blueprints and C++ code. For further information on using and configuring the workspace and the Unreal Engine project, refer to the [PubNub Unreal SDK documentation](https://www.pubnub.com/docs/sdks/unreal).

![Using docs](readme_content/using_docs.gif)

You can copy the Blueprints from the docs and paste them into your Unreal Editor.

## Security

### Keys and credentials

PubNub provides three keys per keyset:

| Key | Safe for client builds | Purpose |
| --- | --- | --- |
| Publish Key | Yes | Publish messages |
| Subscribe Key | Yes | Subscribe to channels |
| Secret Key | **No** | Root/admin access to the entire keyset |

The **Secret Key grants unrestricted permissions** on your keyset — publish on any channel, read history, mint or revoke PAM tokens, and modify App Context data. **Never ship a Secret Key in a game client build.**

Global plugin settings (`Project Settings → Pubnub SDK`) store only the Publish Key and Subscribe Key. These are safe to include in `DefaultEngine.ini` and packaged client builds.

### Recommended architecture

**Game clients (shipped builds):**

1. Configure only Publish Key and Subscribe Key (via plugin settings or `FPubnubConfig`).
2. Obtain a scoped PAM v3 access token from your backend.
3. Call `SetAuthToken` on the client before subscribing or publishing.

**Dedicated servers / backend tooling:**

1. Provide the Secret Key in `FPubnubConfig` when calling `CreatePubnubClient` — not in global plugin settings.
2. Load the Secret Key from a secure source (environment variable, server-only config, secrets manager).
3. Use `GrantToken` / `RevokeToken` / `SetSecretKey` only in server-side code.

For more details, see the [PubNub Access Manager documentation](https://www.pubnub.com/docs/general/setup/access-manager).

## Support

If you **need help** or have a **general question**, contact [support@pubnub.com](mailto:support@pubnub.com).

## License

This project is licensed under a [custom MIT license](https://github.com/pubnub/unreal/blob/master/LICENSE). For more details about the license, refer to the [License FAQ](https://www.pubnub.com/docs/sdks/license-faq).
