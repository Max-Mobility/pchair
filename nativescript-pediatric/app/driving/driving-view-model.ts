import { Observable } from "tns-core-modules/data/observable";
import { Prop } from '../obs-prop';
import { Bluetooth, Characteristic, Device, Service } from 'nativescript-bluetooth'
import { Toasty, ToastPosition } from 'nativescript-toasty'
import {
    COMMAND, COMMAND_STR, ACTUATOR_MOVING_DIR, ACTUATOR_MOVING_DIR_STR,
    SPEED_SETTING, SPEED_SETTING_STR, SYSTEM_MODES, SYSTEM_MODES_STR
} from "../enum";

export class DrivingViewModel extends Observable {
    public static SERVICE_UUID: string = "000012ff-0000-1000-8000-00805f9b34fb";

    @Prop() peripheral: Device = null;
    @Prop() characteristic: Characteristic = null;
    @Prop() service: Service = null;
    @Prop() isBusy: boolean = false;
    @Prop() isConnected: boolean = false;
    @Prop() writeTimeoutId: any = null; 

    private _bluetooth: Bluetooth;
    private sendArray: string[] = [];

    private LOW_SPEED_COLOR = "#e09623";
    private MEDIUM_SPEED_COLOR = "#c3f038";
    private HIGH_SPEED_COLOR = "#83ebf6";

    @Prop() highSpeed: number = 100;
    @Prop() mediumSpeed: number = 60;
    @Prop() lowSpeed: number = 20;
    @Prop() maxSpeed: number = this.lowSpeed;
    @Prop() currentSpeed: number = 0;
    @Prop() speedColor: string = this.LOW_SPEED_COLOR;

    constructor() {
        super();
        this._bluetooth = new Bluetooth();
        //this._bluetooth.debug = true;

        this._bluetooth.addEventListener(Bluetooth.device_acl_connected_event, (eventData: any) => {
            console.log('periphral connected: ' + JSON.stringify(eventData.data.device.name));
            this.isConnected = true;
        });

        this._bluetooth.addEventListener(Bluetooth.device_acl_disconnected_event, (eventData: any) => {
            console.log('periphral disconnected: ' + JSON.stringify(eventData.data.device.name));
            this.isConnected = false;
        });
    }

    public async onBluetoothTap() {
        console.log('onBluetoothTap.');
        await this.scanAndConnect();
    }

    public async onDisconnectTap() {
        console.log('onDisconnectTap.');
        await this.disconnect();
    }

    public async scanAndConnect() {
        try {
            this.isBusy = true;
            console.log('scanAndConnect()');

            const gotPermissions = await this._bluetooth.requestCoarseLocationPermission();
            console.log('gotPermissions: ', gotPermissions);

            const peripheral = await new Promise(async (resolve, reject) => {
                await this._bluetooth.startScanning({
                    serviceUUIDs: [DrivingViewModel.SERVICE_UUID],
                    seconds: 10,
                    onDiscovered: (peripheral: any) => {
                        //TODO: peripheral is not a Device.
                        console.log('onDiscovered: ', peripheral);
                        this._bluetooth.stopScanning();
                        resolve(peripheral);
                    }
                });
                reject('Timeout searching for device');
            }) as Device;
            console.log('peripheral: ', peripheral);
            //this.peripheral = peripheral;

            await new Promise(async (resolve, reject) => {
                this._bluetooth.connect({
                    UUID: peripheral.UUID,
                    onConnected: (peripheral: Device) => {
                        this.onConnected(peripheral);
                        resolve(peripheral);
                    },
                    onDisconnected: (peripheral: Device) => {
                        this.onDisconnected(peripheral);
                        reject('Could not connect to' + peripheral);
                    }
                });
            });
            console.log('connected');
            this._bluetooth.sendEvent('peripheral_connected_event');
        } catch (err) {
            const msg = 'Could not scan and connect:\n' + err;
            this.showError(msg);
        }
        this.isBusy = false;
    }
    private showError(msg: string) {
        console.error(msg);
        new Toasty({
            text: msg,
            position: ToastPosition.CENTER
        }).show();
    }
    private async onDisconnected(peripheral: Device) {
        this.peripheral = peripheral;
    }
    private async onConnected(peripheral: Device) {
        this.peripheral = peripheral;
        console.log('onConnected.');
        this.peripheral.services.forEach(async (service) => {
            if (service.UUID == "12ff") {
                console.log('service: ' + service.UUID);
                this.service = service;
                this.service.characteristics.forEach(async (characteristic) => {
                    if (characteristic.UUID == "ff01") {
                        this.characteristic = characteristic;
                        //console.log('characteristic properites read: ' + characteristic.properties.read);
                        //console.log('characteristic properites write: ' + characteristic.properties.write);
                        //console.log('characteristic properites notify: ' + characteristic.properties.notify);
                        if (characteristic.properties.notify) {
                            await this.startNotifyCharacteristic(this.characteristic);
                        }
                    }
                })
            }
        });
    }
    private async startNotifyCharacteristic(characteristic: Characteristic): Promise<any> {
        console.log("startNotify: " + characteristic.name);
        try {
            await this._bluetooth.startNotifying({
                peripheralUUID: this.peripheral.UUID,
                serviceUUID: DrivingViewModel.SERVICE_UUID,
                characteristicUUID: characteristic.UUID,
                onNotify: this.handleNotify.bind(this)
            });
        } catch (err) {
            console.error('could not start notifying', err);
        }
    }

    private async handleNotify(args: any) {
        console.log('onNotify: ' + new Uint8Array(args.value));
        console.log('onNotify: ' + args.valueRaw);
        console.log('onNofity: ' + args.characteristicUUID);


    }

    public async disconnect() {
        try {
            this.isBusy = true;
            console.log('disconnect()');

            this._bluetooth.disconnect({
                UUID: this.peripheral.UUID
            }).then(() => {
                console.log("disconnect successfully.");
            }, (err) => {
                console.log("discounnected error: " + err);
            });
        } catch (err) {
            const msg = 'Could not disconnect:\n' + err;
            this.showError(msg);
        }
        this.isBusy = false;
    }

    highSpeedTap() {

        if (this.isConnected) {
            //console.log('High speed is chosen.');
            this.maxSpeed = this.highSpeed;
            this.speedColor = this.HIGH_SPEED_COLOR;
            this.sendChoice("high_speed");
        }
    }

    mediumSpeedTap() {
        if (this.isConnected) {
            //console.log('Medium speed is chosen.');
            this.maxSpeed = this.mediumSpeed;
            this.speedColor = this.MEDIUM_SPEED_COLOR;
            this.sendChoice("medium_speed");
        }
    }

    lowSpeedTap() {
        if (this.isConnected) {
            //console.log('Low speed is chosen.');
            this.maxSpeed = this.lowSpeed;
            this.speedColor = this.LOW_SPEED_COLOR;
            this.sendChoice("low_speed");
        }
    }

    public async sendChoice(string: string) {
        if (!this.characteristic.properties.write) {
            console.log("Write property is not enabled.");
            return;
        }

        var sendString = null;
        var byteZero: string = null;
        var byteOne: string = null;
        const byteComma: string = ',';
        switch (string) {
            case 'high_speed':
                //console.log("high_speed");
                byteZero = COMMAND_STR.CMD_SET_SPEED_STR;
                byteOne = SPEED_SETTING_STR.SPEED_HIGH_STR;
                break;
            case 'medium_speed':
                //console.log("medium_speed");
                byteZero = COMMAND_STR.CMD_SET_SPEED_STR;
                byteOne = SPEED_SETTING_STR.SPEED_MEDIUM_STR;
                break;
            case 'low_speed':
                //console.log("low_speed");
                byteZero = COMMAND_STR.CMD_SET_SPEED_STR;
                byteOne = SPEED_SETTING_STR.SPEED_LOW_STR;
                break;
            default:
                console.log("Unexpected value" + string);
        }
        sendString = byteZero.concat(byteComma, byteOne);
        console.log('sendString: ' + sendString);

        this.toBluetooth(sendString);
    }

    private async toBluetooth(string: string) {
        
        if(string!=null){
            //console.log('push string: '+string);
            this.sendArray.push(string);
        }
        //console.log('sendArray:'+this.sendArray[0]);
        if(this.writeTimeoutId == null){
            this.writeTimeoutId = setTimeout(this.writeBLE.bind(this), 0);
        }
    }

    private async writeBLE(){
        if(this.sendArray.length > 0) {
            var string : string = this.sendArray.shift();
            //console.log('get string: '+string);
            try {
                this.isBusy=true;
                const writeStatus = await this._bluetooth.write({
                        peripheralUUID: this.peripheral.UUID,
                        serviceUUID: this.service.UUID,
                        characteristicUUID: this.characteristic.UUID,
                        value: string
                    });
                if ( writeStatus.status !== android.bluetooth.BluetoothGatt.GATT_SUCCESS){
                    this.sendArray.unshift(string);
                }
                console.log('write: '+string);
            } catch (err) {
                const msg = 'Could not write bluetooth: ' + err;
                this.showError(msg);
            }
        }
        if(this.sendArray.length > 0){
            this.writeTimeoutId = setTimeout(this.writeBLE.bind(this), 0);
        } else {
            this.writeTimeoutId = null;
            this.isBusy = false;
        }
    }
}
