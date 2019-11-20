import { SelectedIndexChangedEventData, BottomNavigation } from "tns-core-modules/ui/bottom-navigation";
import { PChair, pChair } from "./pchair";
import { SeatingViewModel } from "./seating/seating-view-model";

export function onSelectedIndexChanged(args: SelectedIndexChangedEventData) {
    const oldIndex = args.oldIndex;
    const newIndex = args.newIndex;    
    
    console.log(`Selected index has changed ( Old index: ${args.oldIndex} New index: ${args.newIndex} )`);
    //console.log("object:"+args.object);
    
    if (pChair.isConnected && newIndex ==0){
        console.log("tab change to 0");
        pChair.sendChoice("run");
    }
    
    if(pChair.isConnected && newIndex ==1){
        //console.log("tab change to 1."+view);
        //view.selectItemAt(0);
        if(SeatingViewModel!=null){
            pChair.sendChoice(SeatingViewModel.select);
        }        
    }

}