# Firmware components diagram

```plantuml
@startuml
title UNav2 Firmware - firmware - Components

top to bottom direction

!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4.puml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Context.puml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Component.puml

Container_Boundary("UNav2Firmware.firmware_boundary", "firmware", $tags="") {
  Component(UNav2Firmware.firmware.timersinterface, "timers interface", "", $tags="")
  Component(UNav2Firmware.firmware.encodersinterface, "encoders interface", "", $tags="")
  Component(UNav2Firmware.firmware.motorcontroller, "motor controller", "", $tags="")
  Component(UNav2Firmware.firmware.healthcheck, "health check", "", $tags="")
  Component(UNav2Firmware.firmware.supervisor, "supervisor", "", $tags="")
  Component(UNav2Firmware.firmware.DebugCLI, "Debug CLI", "", $tags="")
  Component(UNav2Firmware.firmware.serial_io, "serial_io", "", $tags="")
  Component(UNav2Firmware.firmware.messagedispatcher, "message dispatcher", "", $tags="")
  Component(UNav2Firmware.firmware.configurationmanager, "configuration manager", "", $tags="")
}

Rel_D(UNav2Firmware.firmware.messagedispatcher, UNav2Firmware.firmware.motorcontroller, "", $tags="")
Rel_D(UNav2Firmware.firmware.motorcontroller, UNav2Firmware.firmware.timersinterface, "set motor output", $tags="")
Rel_D(UNav2Firmware.firmware.encodersinterface, UNav2Firmware.firmware.motorcontroller, "motor position/speed", $tags="")
Rel_D(UNav2Firmware.firmware.configurationmanager, UNav2Firmware.firmware.motorcontroller, "configuration", $tags="")
Rel_D(UNav2Firmware.firmware.configurationmanager, UNav2Firmware.firmware.healthcheck, "", $tags="")
Rel_D(UNav2Firmware.firmware.supervisor, UNav2Firmware.firmware.messagedispatcher, "readyness & status", $tags="")
Rel_D(UNav2Firmware.firmware.healthcheck, UNav2Firmware.firmware.supervisor, "Health status", $tags="")
Rel_D(UNav2Firmware.firmware.serial_io, UNav2Firmware.firmware.DebugCLI, "rx", $tags="")
Rel_D(UNav2Firmware.firmware.DebugCLI, UNav2Firmware.firmware.serial_io, "tx", $tags="")
Rel_D(UNav2Firmware.firmware.messagedispatcher, UNav2Firmware.firmware.supervisor, "", $tags="")
Rel_D(UNav2Firmware.firmware.serial_io, UNav2Firmware.firmware.messagedispatcher, "rx", $tags="")
Rel_D(UNav2Firmware.firmware.messagedispatcher, UNav2Firmware.firmware.serial_io, "tx", $tags="")
Rel_D(UNav2Firmware.firmware.messagedispatcher, UNav2Firmware.firmware.configurationmanager, "", $tags="")

SHOW_LEGEND(true)
@enduml
```