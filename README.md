# HRMCLi

**Homelab Rack Manager Command Line Interface**

HRMCLi is a lightweight, appliance-style utility designed to provide centralized management of servers and other systems through their BMC interfaces.

Rather than functioning as a traditional Linux application installed on a general-purpose operating system, HRMCLi is intended to behave as the primary interface of the system on which it is installed.

Once the system boots, the user is presented with the HRMCLi terminal user interface (TUI). The underlying operating system should rarely, if ever, need to be accessed directly.

HRMCLi is intended for physical appliances, virtual machines, and containers.

---

## Purpose

HRMCLi provides a dedicated management point for homelab infrastructure.

Its primary purpose is to communicate with server Baseboard Management Controllers (BMCs), allowing an administrator to monitor and control systems independently of their primary operating systems.

Typical functionality will include:

* Viewing managed systems
* Checking system and BMC status
* Powering systems on, off, or restarting them
* Viewing hardware information
* Viewing sensor data
* Viewing and clearing system event logs
* Identifying systems using chassis LEDs
* Managing HRMCLi configuration
* Viewing HRMCLi logs
* Managing BMC credentials
* Accessing an HRMCLi command-line interface
* Managing the HRMCLi appliance itself

HRMCLi is particularly intended to remain useful when the primary network or normal management infrastructure is unavailable.

---

## Design Philosophy

HRMCLi follows an appliance-first design.

The user should not need to understand or administer the underlying Linux system in order to operate HRMCLi.

If an ordinary HRMCLi administrator must exit the application and use the Linux shell to perform a normal administrative task, HRMCLi is missing functionality.

### TUI First

The primary HRMCLi interface is a full-screen terminal user interface.

Routine tasks should be available through menus, dialogs, status screens, and other terminal-based interfaces.

Examples include:

* Editing configuration
* Adding or removing managed nodes
* Configuring networking
* Viewing logs
* Changing settings
* Managing updates
* Backing up or restoring configuration

Configuration files may still exist internally, but users should normally interact with them through HRMCLi rather than manually editing them.

### Embedded Command Line

HRMCLi retains its command-line interface as part of the project.

However, the command line is no longer the primary user interface.

Instead, an HRMCLi terminal or command console will be available from within the TUI for users who prefer command-driven administration or require advanced functionality.

### Lightweight

HRMCLi should remain extremely lightweight.

The application should be capable of running comfortably on very limited hardware and should avoid unnecessary services or dependencies.

### Resilient

HRMCLi should remain accessible during infrastructure failures whenever possible.

A dedicated management network and serial console access are therefore strongly encouraged for physical installations.

### Self-Contained Administration

Normal administration should be performed entirely through HRMCLi.

Users should not normally need to:

* Manually edit Linux configuration files
* Run package manager commands
* Modify system services
* Use `journalctl` to inspect HRMCLi logs
* Directly edit HRMCLi databases
* Manually update software dependencies
* Use the Linux shell for routine maintenance

Advanced users may deliberately exit HRMCLi and access the underlying operating system when necessary.

---

## Hardware Requirements

HRMCLi is designed to operate on very modest hardware.

### Minimum

* 1 CPU core
* 512 MB RAM
* 4 GB persistent storage
* 1 Ethernet interface

512 MB of memory is expected to provide considerable headroom for normal HRMCLi operation.

### Recommended

* 1 CPU core or greater
* 1 GB RAM
* 8 GB or more persistent storage
* Dedicated Ethernet interface for BMC communication
* Dedicated management network
* Dedicated management switch
* Serial console access for physical appliances

A second Ethernet interface may be useful when HRMCLi is connected to both a normal administrative network and an isolated BMC management network.

---

## Network Architecture

At minimum, HRMCLi requires an Ethernet connection capable of reaching the BMC interfaces of managed systems.

For installations containing multiple servers, a dedicated management network is recommended.

Example:

```text
                Administration Network
                         |
                    +---------+
                    | HRMCLi  |
                    +----+----+
                         |
                  Management Network
                         |
                +--------+--------+
                | Management      |
                | Switch          |
                +--+----+----+----+
                   |    |    |
                  BMC  BMC  BMC
```

The management network may be completely isolated from the primary network if desired.

A dedicated management switch is recommended for larger installations.

---

## Access Methods

HRMCLi should eventually support several methods of accessing the same interface.

### Local Console

A keyboard and display may be connected directly to a physical HRMCLi appliance.

### Serial Console

Serial access is strongly recommended for physical deployments.

This provides an out-of-band method of reaching HRMCLi even when normal network connectivity is unavailable.

### SSH

HRMCLi may also be accessed remotely over SSH.

Regardless of the connection method, the user should be presented with the same HRMCLi TUI.

---

## User Interface

A typical HRMCLi interface may contain sections similar to:

```text
HRMCLi
├── Dashboard
├── Nodes
│   ├── Status
│   ├── Power
│   ├── Sensors
│   ├── Hardware Information
│   └── Event Logs
├── Configuration
│   ├── Nodes
│   ├── Network
│   ├── Users
│   └── Preferences
├── Logs
├── Terminal
└── System
    ├── Updates
    ├── Backup / Restore
    ├── Networking
    ├── Restart
    ├── Shutdown
    └── Advanced
```

The exact interface will evolve as development progresses.

---

## BMC Management

HRMCLi is intended to support multiple BMC management protocols behind a common internal interface.

### IPMI

IPMI support will provide compatibility with a large amount of existing server hardware and is expected to be one of HRMCLi's primary management protocols.

### Redfish

Redfish support is planned for newer systems that expose modern REST-based BMC management interfaces.

The internal HRMCLi architecture should avoid tying node management directly to a single protocol.

Conceptually:

```text
                HRMCLi
                   |
            BMC Management API
              /           \
           IPMI          Redfish
            |               |
          Server          Server
```

---

## Configuration

HRMCLi configuration should be manageable through the TUI.

Configuration may include:

* Managed nodes
* BMC addresses
* Management protocols
* Network configuration
* User settings
* Interface preferences
* Logging configuration
* Update configuration

Where direct text editing is useful, HRMCLi should provide an integrated text editor rather than requiring the administrator to exit to a Linux shell.

---

## Credentials

BMC credentials must be treated separately from ordinary node configuration.

A node configuration should reference stored credentials rather than containing plaintext usernames and passwords directly.

For example:

```json
{
    "name": "server-01",
    "address": "192.168.100.10",
    "protocol": "ipmi",
    "credential": "cred-0001"
}
```

Credential storage should be designed to minimize unnecessary exposure of sensitive information.

Exporting ordinary configuration should not automatically expose BMC passwords.

---

## Logging

HRMCLi should provide its own log viewer.

Logging categories may include:

* Application events
* BMC communication
* Authentication events
* Administrative actions
* Network events
* System events
* Update events

Administrators should not normally need to use Linux logging tools to diagnose HRMCLi.

---

## Filesystem Philosophy

The HRMCLi operating environment should distinguish between the system image and persistent application data.

The base operating system and HRMCLi application should eventually be considered replaceable.

Persistent data should survive application or system upgrades.

Persistent data may include:

* Configuration
* Node definitions
* Credentials
* Logs
* Backups
* Application state

This separation will allow HRMCLi to eventually use appliance-style updates rather than traditional manual operating-system administration.

A future deployment may use a mostly read-only system filesystem with dedicated writable storage for HRMCLi data.

---

## Updates

Users should not be expected to manually update the underlying Linux operating system.

Instead, HRMCLi should eventually provide appliance-style update management through the TUI.

Possible functionality includes:

* Check for updates
* Download updates
* Verify updates
* Install updates
* View installed version
* View update history
* Roll back a failed update

The implementation of the underlying operating system should remain largely invisible to the user.

---

## Backup and Recovery

HRMCLi should provide integrated backup and restore functionality.

Backups may include:

* Node configuration
* HRMCLi settings
* Network configuration
* Credential data
* User configuration

Possible operations include:

* Create backup
* Restore backup
* Export configuration
* Import configuration
* Factory reset

Sensitive backup data should be protected appropriately.

---

## Software Architecture

HRMCLi will be developed primarily in C.

The project will be divided into independent subsystems rather than implementing all functionality directly inside the user interface.

A high-level architecture may resemble:

```text
HRMCLi
│
├── Application Core
│
├── TUI
│   ├── Dashboard
│   ├── Nodes
│   ├── Configuration
│   ├── Logs
│   ├── Terminal
│   └── System
│
├── Configuration
│
├── Logging
│
├── Node Management
│
├── Credential Management
│
├── BMC Layer
│   ├── IPMI
│   └── Redfish
│
├── Networking
│
├── Backup / Recovery
│
├── Update System
│
└── Embedded CLI
```

The TUI should consume these subsystems rather than contain their implementation directly.

---

## Operating Environment

HRMCLi is intended to eventually operate as a dedicated utility environment.

Supported deployment targets may include:

* Dedicated physical appliance
* Raspberry Pi
* Small x86 system
* Virtual machine
* Linux container

The underlying Linux distribution should ultimately be an implementation detail rather than something the administrator routinely interacts with.

---

## Project Status

HRMCLi is currently undergoing a ground-up architectural rewrite.

An earlier version of the project implemented a custom command-line shell and preliminary node-management functionality.

The new version is transitioning HRMCLi into a complete TUI-driven appliance while retaining an integrated command-line interface for advanced use.

Current development priorities are:

1. Define project architecture
2. Establish the new source tree
3. Build the initial TUI framework
4. Implement configuration management
5. Implement logging
6. Implement node management
7. Create the BMC abstraction layer
8. Implement IPMI support
9. Implement Redfish support
10. Reintroduce the HRMCLi command interface
11. Develop the appliance operating environment
12. Implement updates, backups, and recovery

---

## License

See [LICENSE](LICENSE) for licensing information.
