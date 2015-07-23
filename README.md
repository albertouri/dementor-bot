Simple bot based on agents, state machines, potential fields and basic reactive planning. Created in about three weeks of human time (mostly testing). Participant and winner of SSCAI 2012.

Bot's source code is a mess, but it works, almost without any crashes or lags. It's in state as it was submitted to SSCAI 2012. Main logic is in Commander.cpp file. If you are interested in agent logic, look at VultureAgent.cpp or TankAgent.cpp and AgentManager.cpp. Attack target planning, scouting targets and map state is managed by MapStateManager.cpp.

Used BWSAL is quick ported code from official project build. It's not working properly all the time and it contains some very dirty hacks and fixes.
