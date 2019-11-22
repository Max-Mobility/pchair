import { Observable } from "tns-core-modules/data/observable";
import { Prop } from '../obs-prop';
import { Toasty, ToastPosition } from 'nativescript-toasty'
import { COMMAND } from "../enum";
import { PChair, pChair } from '../pchair';



export class DrivingViewModel extends Observable {

    //public static SERVICE_UUID: string = "000012ff-0000-1000-8000-00805f9b34fb";

    //public static notifyValue;

    // move to pchair
    // @Prop() public peripheral: Device = null;
    // @Prop() public characteristic: Characteristic = null;
    // @Prop() public service: Service = null;
    // @Prop() isBusy: boolean = false;
    @Prop() isConnected: boolean = false;
    // @Prop() writeTimeoutId: any = null;

    // public _bluetooth: Bluetooth;
    // private sendArray: string[] = [];
    //public oldIndex: number;
    //public newIndex: number;

    private pChair: PChair;
    private LOW_SPEED_COLOR = "#e09623";
    private MEDIUM_SPEED_COLOR = "#c3f038";
    private HIGH_SPEED_COLOR = "#83ebf6";

    @Prop() highSpeed: number = 15;
    @Prop() mediumSpeed: number = 10.5;
    @Prop() lowSpeed: number = 6;
    @Prop() maxSpeed: number = this.lowSpeed;
    @Prop() currentSpeed: number = 0;
    @Prop() speedColor: string = this.LOW_SPEED_COLOR;

    constructor() {
        super();
        this.pChair = pChair;
        this.registerEvents();
    }
    private unregisterEvents() {
        this.pChair.off(
            PChair.pchair_connect_event,
            this.onPChairConnect,
            this
        );
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
        this.pChair.on(
            PChair.pchair_connect_event,
            this.onPChairConnect,
            this
        );
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

    public sendMaxSpeed() {
        if (this.maxSpeed == this.highSpeed) {
            this.pChair.sendChoice("high_speed");
        } else if (this.maxSpeed == this.mediumSpeed) {
            this.pChair.sendChoice("medium_speed");
        } else if (this.maxSpeed == this.lowSpeed) {
            this.pChair.sendChoice("low_speed");
        }
    }

    private onPChairConnect() {
        console.log("pchair_connect_event Event");
        this.isConnected = true;
        this.pChair.sendChoice("run");

        if (this.maxSpeed == this.highSpeed) {
            this.highSpeedTap();
        } else if (this.maxSpeed == this.mediumSpeed) {
            this.mediumSpeedTap();
        } else if (this.maxSpeed == this.lowSpeed) {
            this.lowSpeedTap();
        }
    }
    private onPChairDisconnect() {
        console.log("pchair_disconnect_event Event");
        this.isConnected = false;

        new Toasty({
            text: "Bluetooth disconnected",
            position: ToastPosition.CENTER
        }).show();
    }

    private onPChairNotify(args: any) {
        const command = args.data.Command;
        const value = args.data.Value;
        //console.log("Notify event: "+ command + " " + value);

        if (command == COMMAND.CMD_DRIVE_SPEED) {
            this.currentSpeed = Math.min(Math.max(value, 0), 100) * 0.15;
        }
    }

    public async onBluetoothTap() {
        console.log('onBluetoothTap.');
        await this.pChair.scanAndConnect();
    }

    public async onDisconnectTap() {
        console.log('onDisconnectTap.');
        await this.pChair.disconnect();
    }

    private highSpeedTap() {

        if (this.isConnected) {
            console.log('High speed is chosen.');
            this.maxSpeed = this.highSpeed;
            this.speedColor = this.HIGH_SPEED_COLOR;
            this.pChair.sendChoice("high_speed");
        }
    }

    private mediumSpeedTap() {
        if (this.isConnected) {
            console.log('Medium speed is chosen.');
            this.maxSpeed = this.mediumSpeed;
            this.speedColor = this.MEDIUM_SPEED_COLOR;
            this.pChair.sendChoice("medium_speed");
        }
    }

    private lowSpeedTap() {
        if (this.isConnected) {
            console.log('Low speed is chosen.');
            this.maxSpeed = this.lowSpeed;
            this.speedColor = this.LOW_SPEED_COLOR;
            this.pChair.sendChoice("low_speed");
        }
    }
}
