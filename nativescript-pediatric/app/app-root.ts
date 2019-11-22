import { SelectedIndexChangedEventData, BottomNavigation } from "tns-core-modules/ui/bottom-navigation";
import { PChair, pChair } from "./pchair";
import { COMMAND, SYSTEM_MODES } from "./enum";
import { DrivingViewModel } from "./driving/driving-view-model";

export var bottomNav: BottomNavigation;

export function onSelectedIndexChanged(args: SelectedIndexChangedEventData) {
    const oldIndex = args.oldIndex;
    const newIndex = args.newIndex;

    //console.log(`Selected index has changed ( Old index: ${args.oldIndex} New index: ${args.newIndex} )`);
    //console.log("object:"+args.object);

    if (pChair.isConnected && newIndex == 0) {
        //console.log("tab change to 0");

        pChair.sendChoice("run");
    }

    // if(pChair.isConnected && newIndex ==1){
    //     //console.log("tab change to 1."+view);
    //     //view.selectItemAt(0);
    //     if(SeatingViewModel!=null){
    //         pChair.sendChoice(SeatingViewModel.select);
    //     }        
    // }

    if (pChair.isConnected && newIndex == 2) {
        //console.log("tab change to 2");
        pChair.sendChoice("gokart");
    }

    if (pChair.isConnected && newIndex == 3) {
        //console.log("tab change to 3");
        pChair.sendChoice("sleep");
    }

}

export function onload(args: any) {
    bottomNav = args.object;
    //console.log("onload");

    pChair.off(
        PChair.pchair_notify_event,
        onPChairNotify
    );

    pChair.on(
        PChair.pchair_notify_event,
        onPChairNotify
    );
}

function onPChairNotify(args: any) {
    const command = args.data.Command;
    const value = args.data.Value;
    if (command == COMMAND.CMD_CHANGE_SYSTEM_MODE) {
        console.log("change tab.");
        switch (value) {
            case SYSTEM_MODES.DRIVEMODE:
                bottomNav.selectedIndex = 0;
                break;
            case SYSTEM_MODES.PHONE_CONTROL_MODE:
                bottomNav.selectedIndex = 2;
                break;
            case SYSTEM_MODES.SYSTEM_SLEEP:
                bottomNav.selectedIndex = 3;
                break;
            case SYSTEM_MODES.ACTUATOR_RECLINE:
                bottomNav.selectedIndex = 1;
                break;
            default:
                console.log("invalid");
        }
    }
}