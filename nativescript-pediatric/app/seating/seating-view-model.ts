import { ObservableArray } from 'tns-core-modules/data/observable-array';
import { Observable, fromObject, EventData } from 'tns-core-modules/data/observable';
import { Bluetooth, Characteristic, Device, Service } from 'nativescript-bluetooth';
import { Toasty, ToastPosition } from 'nativescript-toasty';
import { ListViewEventData, RadListView } from 'nativescript-ui-listview';
import { Prop } from '../obs-prop';
import { ControlItem } from './control-item';
import { PChair, pChair } from '../pchair';
import { DrivingViewModel } from '~/driving/driving-view-model';
import { view } from './seating-page'
import {
    COMMAND, COMMAND_STR, ACTUATOR_MOVING_DIR, ACTUATOR_MOVING_DIR_STR,
    SPEED_SETTING, SPEED_SETTING_STR, SYSTEM_MODES, SYSTEM_MODES_STR
} from "../enum";
import { getFrameById, getViewById, ViewBase } from 'tns-core-modules/ui/frame/frame';
import { TouchGestureEventData } from 'tns-core-modules/ui/gestures/gestures';

export class SeatingViewModel extends Observable {
    @Prop() controlItems: ObservableArray<ControlItem> = new ObservableArray();
    @Prop() public selectedControl: ControlItem = null;

    static select: string = null;

    private pChair: PChair;
    // private view : RadListView;

    constructor() {
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
        this.selectedControl = this.controlItems.getItem(0);
        this.selectedControl.selected = true;

        SeatingViewModel.select = this.selectedControl.name;
        if (pChair.isConnected) {
            this.pChair.sendChoice(SeatingViewModel.select);
        }


        this.registerEvents();
    }

    private unregisterEvents() {
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

    private onPChairNotify(args: any) {
        const command = args.data.Command;
        const value = args.data.Value;

        if (command == COMMAND.CMD_CHANGE_SYSTEM_MODE) {
            switch (value) {
                case SYSTEM_MODES.ACTUATOR_RECLINE:
                    this.changeSelection(0);
                    view.selectItemAt(0);
                    break;
                case SYSTEM_MODES.ACTUATOR_LEGREST:
                    this.changeSelection(1);
                    view.selectItemAt(1);
                    break;
                case SYSTEM_MODES.ACTUATOR_TILT:
                    this.changeSelection(2);
                    view.selectItemAt(2);
                    break;
                case SYSTEM_MODES.ACTUATOR_ELEVATION:
                    this.changeSelection(3);
                    view.selectItemAt(3);
                    break;
                case SYSTEM_MODES.ACTUATOR_STAND:
                    this.changeSelection(4);
                    view.selectItemAt(4);
                    break;
                default:
                    break;
            }
        }
    }

    private changeSelection(index: number) {
        this.controlItems.map(e => {
            e.selected = false;
        });
        this.selectedControl = this.controlItems.getItem(index);
        this.selectedControl.selected = true;
        SeatingViewModel.select = this.selectedControl.name;

    }

    private onPChairDisconnect() {
        console.log("pchair_disconnect_event Event");

        new Toasty({
            text: "Bluetooth disconnected",
            position: ToastPosition.CENTER
        }).show();
    }

    public async onControlSelected(event: ListViewEventData) {
        //console.log('onControlTap()');
        // const view=event.object;
        // view.selectItemAt(event.index);
        this.changeSelection(event.index);
        if (pChair.isConnected) {
            pChair.sendChoice(SeatingViewModel.select);
            //console.log('control:', this.selectedControl.name);
        }

    }

    public async onIncreaseControl(args: TouchGestureEventData) {
        switch (args.action) {
            case "down":
                pChair.sendChoice("plus_down");
                break;
            case "up":
                pChair.sendChoice("stop");
                break;
            default:
            //console.log('unexpected value.');
        }
    }

    public async onDecreaseControl(args: TouchGestureEventData) {
        switch (args.action) {
            case "down":
                pChair.sendChoice("minus_down");
                break;
            case "up":
                pChair.sendChoice("stop");
                break;
            default:
            //console.log('unexpected value.');
        }
    }
}
