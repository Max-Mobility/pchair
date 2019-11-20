import { ObservableArray } from 'tns-core-modules/data/observable-array';
import { Observable, fromObject } from 'tns-core-modules/data/observable';
import { Bluetooth, Characteristic, Device, Service } from 'nativescript-bluetooth';
import { Toasty, ToastPosition } from 'nativescript-toasty';
import { ListViewEventData } from 'nativescript-ui-listview';
import { Prop } from '../obs-prop';
import { ControlItem } from './control-item';
import { PChair,pChair } from '../pchair';
import { DrivingViewModel } from '~/driving/driving-view-model';

import {
    COMMAND, COMMAND_STR, ACTUATOR_MOVING_DIR, ACTUATOR_MOVING_DIR_STR,
    SPEED_SETTING, SPEED_SETTING_STR, SYSTEM_MODES, SYSTEM_MODES_STR
} from "../enum";
import { getFrameById, getViewById, ViewBase } from 'tns-core-modules/ui/frame/frame';

export class SeatingViewModel extends Observable {
    @Prop() controlItems: ObservableArray<ControlItem> = new ObservableArray();
    @Prop() public selectedControl: ControlItem = null;

    static select: string = null;

    private pChair: PChair;

    private startEvent = {
        eventname: "start",
        index: 0
    };


    constructor (){ 
        super();
        this.pChair = pChair;

        this.controlItems.push([
            new ControlItem('Recline', '~/assets/images/back.png'),
            new ControlItem('Legs', '~/assets/images/leg.png'),
            new ControlItem('Tilt', '~/assets/images/tilt.png'),
            new ControlItem('Elevation', '~/assets/images/elev.png'),
            new ControlItem('Stand', '~/assets/images/stand.png'),
            //new ControlItem('Go-Kart Function', '~/assets/images/go.png')
        ]);
        //this.selectedControl = this.controlItems.getItem(0);
        //this.selectedControl.selected = true;

        this.onControlSelected(this.startEvent);

        SeatingViewModel.select=this.selectedControl.name;
        if(pChair.isConnected){
            this.pChair.sendChoice(SeatingViewModel.select);
        }
        

        this.registerEvents();
    }

    private unregisterEvents(){
        // this.pChair.off(
        //     PChair.pchair_connect_event,
        //     this.onPChairConnect,
        //     this
        // );
        this.pChair.off(
            PChair.pchair_disconnect_event,
            this.onPChairDisconnect,
            this
        );
        this.pChair.off(
            PChair.pchair_notify_event,
            this.onPChairNotify,
            this
        );
    }

    private registerEvents() {
        this.unregisterEvents();
        // this.pChair.on(
        //     PChair.pchair_connect_event,
        //     this.onPChairConnect,
        //     this
        // );
        this.pChair.on(
            PChair.pchair_disconnect_event,
            this.onPChairDisconnect,
            this
        );
        this.pChair.on(
            PChair.pchair_notify_event,
            this.onPChairNotify,
            this
        );
    }

    private onPChairNotify(args: any){
        const command = args.data.Command;
        const value = args.data.Value;
        
        // if(command == COMMAND.CMD_CHANGE_SYSTEM_MODE){
        //     switch(value){
        //         case SYSTEM_MODES.ACTUATOR_RECLINE:
        //             this.onControlSelected
        //     }
        // }
        // switch (command) {
        //     case 0:
        //         //console.log("Target speed: "+value);
        //         // set speed in [0,100]
        //         this.currentSpeed = Math.min(Math.max(value,0),100);
        //         break;
        //     default:
        //         //console.log("Unexpected value" + args);
        // }
    }

    private onPChairDisconnect(){
        console.log("pchair_disconnect_event Event");

        new Toasty({
            text: "Bluetooth disconnected",
            position: ToastPosition.CENTER
        }).show();
    }

    public async onControlSelected(event: ListViewEventData) {
        console.log('onControlTap()');
        this.controlItems.map(e => {
            e.selected = false;
        });

        // const view=event.object;
        // view.selectItemAt(event.index);
        this.selectedControl = this.controlItems.getItem(event.index);
        this.selectedControl.selected = true;
        SeatingViewModel.select = this.selectedControl.name;
        pChair.sendChoice(SeatingViewModel.select);
        //console.log('control:', this.selectedControl.name);
    }
}
