# UNav2 System diagram

```plantuml
@startuml
title UNav2 Firmware - System Context

top to bottom direction

!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4.puml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Context.puml

System(name, "name:", "ROS Control HW Interface", $tags="")
System(UNav2Firmware, "UNav2 Firmware", "UNav2 Firmware", $tags="")
Person(Dev, "Dev", "Developer", $tags="")

Rel_D(name, UNav2Firmware, "control", "protobuf", $tags="")
Rel_D(name, UNav2Firmware, "configure", "protobuf", $tags="")
Rel_D(UNav2Firmware, name, "feedback", "protobuf", $tags="")
Rel_D(Dev, UNav2Firmware, "Debug cli interface", "serial", $tags="")

SHOW_LEGEND(true)
@enduml
```
