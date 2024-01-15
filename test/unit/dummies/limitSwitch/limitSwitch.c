#define SOURCE_FILE "LIMIT_SWITCH"

#include "limitSwitch.h"

bool re = true;
bool limitSwitchIsClosed(limitSwitch_t limitSwitch) {
    re = !re;
    return re;
}

limitSwitch_t createLimitSwitch(uint8_t id) {
    limitSwitch_t limitSwitch;

    switch (id) {
    case 0:
        limitSwitch = LIMIT_SWITCH_0;
        break;
    case 1:
        limitSwitch = LIMIT_SWITCH_1;
        break;
    case 2:
        limitSwitch = LIMIT_SWITCH_2;
        break;
    case 3:
        limitSwitch = LIMIT_SWITCH_3;
        break;
    default:
        return (limitSwitch_t){.pin = -1};
    }

    return limitSwitch;
}
