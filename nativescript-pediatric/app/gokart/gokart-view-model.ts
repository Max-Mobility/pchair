import { Observable } from "tns-core-modules/data/observable";
import { Prop } from '../obs-prop';
import { Toasty, ToastPosition } from 'nativescript-toasty'
import { COMMAND } from "../enum";
import { PChair, pChair } from '../pchair';
import { Sensor } from "../sensor"

export class GokartViewModel extends Observable {

    private pChair: PChair;
    private LOW_SPEED_COLOR = "#e09623";
    private MEDIUM_SPEED_COLOR = "#c3f038";
    private HIGH_SPEED_COLOR = "#83ebf6";

    private arrayXY: Array<number> = [];
    private arrayZ: Array<number> = [];
    private zeroXY: number;
    private zeroZ: number;


    @Prop() highSpeed: number = 15;
    @Prop() mediumSpeed: number = 10.5;
    @Prop() lowSpeed: number = 6;
    @Prop() maxSpeed: number = this.lowSpeed;
    @Prop() currentSpeed: number = 0;
    @Prop() speedColor: string = this.LOW_SPEED_COLOR;

    @Prop() isCalibrated: boolean = false;
    @Prop() isBusy: boolean = false;
    
    private mySensor = new Sensor();
    private calibrateTimer: any;

    public clearCalibration() {
        this.isCalibrated = false;
        clearTimeout(this.calibrateTimer);
    }

    public async stopSensor() {
        if (this.mySensor != null) {
            this.mySensor.stopAcc();

            this.mySensor.off(
                Sensor.sensor_gravity_event,
                this.onSensorData,
                this
            );
        }
        this.isBusy = false;
    }

    constructor() {
        super();

        this.pChair = pChair;
        this.registerEvents();
    }

    public async calibrate() {
        this.isBusy = true;
        this.isCalibrated = false;
        console.log("calibrate.");
        this.pChair.sendGoKart(128, 128);

        this.mySensor.stopAcc();

        this.mySensor.off(
            Sensor.sensor_gravity_event,
            this.onSensorData,
            this
        );

        this.mySensor.on(
            Sensor.sensor_gravity_event,
            this.onSensorData,
            this
        );

        // Calibrate after 5 sec
        this.mySensor.startAcc();
        this.calibrateTimer=setTimeout(() => {
            this.zeroXY = this.arrayXY.reduce((sum, current) => sum + current, 0) / this.arrayXY.length;
            this.zeroZ = this.arrayZ.reduce((sum, current) => sum + current, 0) / this.arrayZ.length;
            //console.log("zero: " + this.zeroXY + " " + this.zeroZ);
            this.isCalibrated = true;
            this.isBusy = false;
        }, 5000);
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
        this.mySensor.off(
            Sensor.sensor_gravity_event,
            this.onSensorData,
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
        this.mySensor.on(
            Sensor.sensor_gravity_event,
            this.onSensorData,
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

    private onSensorData(result: any) {
        //console.log(result.data.x);
        const x = result.data.x;
        const y = result.data.y;
        const z = result.data.z;

        const angleZ = Math.acos(z / 10) / Math.PI * 180;
        const angleXY = Math.atan(x / y) / Math.PI * 180;


        if (!this.isCalibrated) {
            this.aPush(angleXY, angleZ);
        } else {
            const deltaZ = this.zeroZ - angleZ;
            const deltaXY = this.zeroXY - angleXY;
            
            if(Math.abs(deltaZ)>20.0 || Math.abs(deltaXY)>25.0){
                if (pChair.isConnected){
                    this.pChair.sendGoKart(128,128);
                }
            } else{
                const sendZ = Math.min(Math.max(deltaZ, -15.0), 15.0) / 15.0 * 127 + 128;
                const sendXY = Math.min(Math.max(deltaXY, -20.0), 20.0) / 20.0 * 127 + 128;
                //console.log("delta: "+ sendXY +" "+sendZ);
                if (pChair.isConnected) {
                    this.pChair.sendGoKart(sendXY, sendZ);
                }
            }    
        }

        //console.log("arrayXY: " + angleZ);
        //console.log("arrayZ: "+this.arrayZ);

        // if (this.isCalibrated) {
        //     const deltaZ = this.zeroZ - angleZ;
        //     const deltaXY = this.zeroXY - angleXY;


        //     const sendZ = Math.min(Math.max(deltaZ, -15.0), 15.0) / 15.0 * 127 + 128;
        //     const sendXY = Math.min(Math.max(deltaXY, -20.0), 20.0) / 20.0 * 127 + 128;

        //     //console.log("delta: "+ sendXY +" "+sendZ);

        //     if (pChair.isConnected) {
        //         this.pChair.sendGoKart(sendXY, sendZ);
        //     }
        // }
    }

    private aPush(angleXY: number, angleZ: number) {
        this.arrayXY.push(angleXY);
        this.arrayZ.push(angleZ);
        if (this.arrayXY.length > 25) {
            this.arrayXY.shift();
            this.arrayZ.shift();
        }
    }


    private onPChairNotify(args: any) {
        const command = args.data.Command;
        const value = args.data.Value;
        //console.log("Notify event: "+ command + " " + value);

        if (command == COMMAND.CMD_DRIVE_SPEED) {
            this.currentSpeed = Math.min(Math.max(value, 0), 100) * 0.15;
        }
    }

    private onPChairDisconnect() {
        console.log("pchair_disconnect_event Event");

        new Toasty({
            text: "Bluetooth disconnected",
            position: ToastPosition.CENTER
        }).show();
    }

    public goKartHigh() {

        if (pChair.isConnected) {
            console.log('Gokart high speed is chosen.');
            this.maxSpeed = this.highSpeed;
            this.speedColor = this.HIGH_SPEED_COLOR;
            this.pChair.sendChoice("high_speed");
        }
    }

    public goKartMedium() {
        if (pChair.isConnected) {
            console.log('Gokart medium speed is chosen.');
            this.maxSpeed = this.mediumSpeed;
            this.speedColor = this.MEDIUM_SPEED_COLOR;
            this.pChair.sendChoice("medium_speed");
        }
    }

    public goKartLow() {
        if (pChair.isConnected) {
            console.log('Gokart low speed is chosen.');
            this.maxSpeed = this.lowSpeed;
            this.speedColor = this.LOW_SPEED_COLOR;
            this.pChair.sendChoice("low_speed");
        }
    }
}