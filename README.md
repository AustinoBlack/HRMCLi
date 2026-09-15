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

