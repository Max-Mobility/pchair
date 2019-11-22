import { Observable } from "tns-core-modules/data/observable";
import { Prop } from './obs-prop';
import { Toasty, ToastPosition } from 'nativescript-toasty'
import { Bluetooth, Characteristic, Device, Service } from 'nativescript-bluetooth'
import { DeviceBase } from './device-base';
import {
    COMMAND, COMMAND_STR, ACTUATOR_MOVING_DIR, ACTUATOR_MOVING_DIR_STR,
    SPEED_SETTING, SPEED_SETTING_STR, SYSTEM_MODES, SYSTEM_MODES_STR
} from "./enum";


export class PChair extends DeviceBase {
    public static SERVICE_UUID: string = "000012ff-0000-1000-8000-00805f9b34fb";
    public static SERVICE_SHORT: string = "12ff";
    public static CHARACTERISTIC_SHORT: string = "ff01";

    // Event names
    public static pchair_connect_event = 'pchair_connect_event';
    public static pchair_disconnect_event = 'pchair_disconnect_event';
    public static pchair_notify_event = 'pchair_notify_event';
    public static pchair_error_event = 'pchair_error_event';

    @Prop() public peripheral: Device = null;
    @Prop() public characteristic: Characteristic = null;
    @Prop() public service: Service = null;
    @Prop() notifyValue: Uint8Array = null;
    @Prop() isBusy: boolean = false;
    @Prop() isConnected: boolean = false;
    @Prop() writeTimeoutId: any = null;

    public _bluetooth: Bluetooth;
    private sendArray: Array<Array<number>> = [];

    constructor() {
        super();
        this._bluetooth = new Bluetooth();
        //this._bluetooth.debug = true;

        this._bluetooth.off(Bluetooth.device_acl_connected_event);
        this._bluetooth.off(Bluetooth.device_acl_disconnected_event);

        this._bluetooth.addEventListener(Bluetooth.device_acl_connected_event, (eventData: any) => {
            console.log('periphral connected: ' + JSON.stringify(eventData.data.device.name));
            this.isConnected = true;
        });

        this._bluetooth.addEventListener(Bluetooth.device_acl_disconnected_event, (eventData: any) => {
            console.log('periphral disconnected: ' + JSON.stringify(eventData.data.device.name));
            this.isConnected = false;
        });
    }

    public async scanAndConnect() {
        try {
            this.isBusy = true;
            console.log('scanAndConnect()');

            const gotPermissions = await this._bluetooth.requestCoarseLocationPermission();
            console.log('gotPermissions: ', gotPermissions);

            const peripheral = await new Promise(async (resolve, reject) => {
                await this._bluetooth.startScanning({
                    serviceUUIDs: [PChair.SERVICE_UUID],
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

    private async onConnected(peripheral: Device) {
        this.peripheral = peripheral;
        console.log('onConnected.');
        this.peripheral.services.forEach(async (service) => {
            if (service.UUID == PChair.SERVICE_SHORT) {
                console.log('service: ' + service.UUID);
                this.service = service;
                this.service.characteristics.forEach(async (characteristic) => {
                    if (characteristic.UUID == PChair.CHARACTERISTIC_SHORT) {
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
                serviceUUID: PChair.SERVICE_UUID,
                characteristicUUID: characteristic.UUID,
                onNotify: this.handleNotify.bind(this)
            });
            this.sendEvent(PChair.pchair_connect_event);
        } catch (err) {
            console.error('could not start notifying', err);
        }
    }

    private async handleNotify(args: any) {
        this.notifyValue = new Uint8Array(args.value);
        var nValue = this.notifyValue;
        var uuid = args.characteristicUUID;
        //console.log('onNotify value: ' + nValue[0] + " " + nValue[1]);
        //console.log('onNotify: ' + args.valueRaw);
        //console.log('onNofity uuid: ' + uuid.toString().substr(4,4));
        

        // TODO: add sendevent
        this.sendEvent(PChair.pchair_notify_event, {
            Command: nValue[0],
            Value: nValue[1]
        });
    }

    private async onDisconnected(peripheral: Device) {
        this.peripheral = peripheral;
    }

    private showError(msg: string) {
        console.error(msg);
        new Toasty({
            text: msg,
            position: ToastPosition.CENTER
        }).show();
    }
    
    public async disconnect() {
        try {
            this.isBusy = true;
            console.log('disconnect(): '+this.characteristic.UUID);

            await this._bluetooth.stopNotifying({
                peripheralUUID: this.peripheral.UUID,
                serviceUUID: PChair.SERVICE_UUID,
                characteristicUUID: this.characteristic.UUID
            }).then(function () {
                console.log('unsubscribed for notification.');
            }, function (err) {
                console.log("unsubscribe error: " + err);
            })

            await this._bluetooth.disconnect({
                UUID: this.peripheral.UUID
            }).then(() => {
                console.log("disconnect successfully.");
                this.sendEvent(PChair.pchair_disconnect_event);
            }, (err) => {
                console.log("discounnected error: " + err);
            });
        } catch (err) {
            const msg = 'Could not disconnect:\n' + err;
            this.showError(msg);
        }
        this.isBusy = false;
    }

    public async sendGoKart(sendXY: number, sendZ: number) {
        if (!this.characteristic.properties.write) {
            console.log("Write property is not enabled.");
            return;
        }

        var byteZero = COMMAND.CMD_PHONE_JOYX;
        var byteOne=sendXY;   

        var sendString = [byteZero, byteOne];
        //console.log('sendString: ' + sendString);

        this.toBluetooth(sendString);

        byteZero = COMMAND.CMD_PHONE_JOYY;
        byteOne=sendZ;   
        sendString = [byteZero, byteOne];
        //console.log('sendString: ' + sendString);
        this.toBluetooth(sendString);
    }

    public async sendChoice(string: string) {
        if (!this.characteristic.properties.write) {
            console.log("Write property is not enabled.");
            return;
        }

        var byteZero: number;
        var byteOne: number;
        
        switch (string) {
            case 'run':
                //console.log("run");
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.DRIVEMODE;
                break;
            case 'high_speed':
                //console.log("high_speed");
                byteZero = COMMAND.CMD_SET_SPEED;
                byteOne = SPEED_SETTING.SPEED_HIGH;
                break;
            case 'medium_speed':
                //console.log("medium_speed");
                byteZero = COMMAND.CMD_SET_SPEED;
                byteOne = SPEED_SETTING.SPEED_MEDIUM;
                break;
            case 'low_speed':
                //console.log("low_speed");
                byteZero = COMMAND.CMD_SET_SPEED;
                byteOne = SPEED_SETTING.SPEED_LOW;
                break;
            case "Recline":
                //console.log("Recline");
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.ACTUATOR_RECLINE;
                break;
            case "Legs":
                //console.log("Legs");
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.ACTUATOR_LEGREST;
                break;
            case "Tilt":
                //console.log("Tilt");
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.ACTUATOR_TILT;
                break;
            case "Elevation":
                //console.log("Elevation");
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.ACTUATOR_ELEVATION;
                break;
            case "Stand":
                //console.log("Stand");
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.ACTUATOR_STAND;
                break;
            case "plus_down":
                //console.log("plus_down");
                byteZero = COMMAND.CMD_MOVE_ACTUATOR;
                byteOne = ACTUATOR_MOVING_DIR.AMD_RIGHT;
                break;
            case "minus_down":
                byteZero = COMMAND.CMD_MOVE_ACTUATOR;
                byteOne = ACTUATOR_MOVING_DIR.AMD_LEFT;
                break;
            case "stop":
                byteZero = COMMAND.CMD_MOVE_ACTUATOR;
                byteOne = ACTUATOR_MOVING_DIR.AMD_STOP;
                break;
            case "sleep":
                byteZero =  COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.SYSTEM_SLEEP;
                break;
            case "gokart":
                byteZero = COMMAND.CMD_CHANGE_SYSTEM_MODE;
                byteOne = SYSTEM_MODES.PHONE_CONTROL_MODE;
                break;
            default:
                console.log("Unexpected value" + string);
        }
        const sendString = [ byteZero, byteOne ];
        //console.log('sendString: ' + sendString);

        this.toBluetooth(sendString);
    }

    private async toBluetooth(string: Array<number>) {

        if (string != null) {
            //console.log('push string: '+string);
            this.sendArray.push(string);
        }
        //console.log('sendArray:'+this.sendArray[0]);
        if (this.writeTimeoutId == null) {
            this.writeTimeoutId = setTimeout(this.writeBLE.bind(this), 0);
        }
    }

    private async writeBLE() {
        if (this.sendArray.length > 0) {
            const value = this.sendArray.shift()
            var string=new Uint8Array(value);
            //console.log('get string: '+string);
            try {
                this.isBusy = true;
                const writeStatus = await this._bluetooth.write({
                    peripheralUUID: this.peripheral.UUID,
                    serviceUUID: this.service.UUID,
                    characteristicUUID: this.characteristic.UUID,
                    value: string
                });
                if (writeStatus.status !== android.bluetooth.BluetoothGatt.GATT_SUCCESS) {
                    this.sendArray.unshift(value);
                }
                console.log('write: ' + string);
            } catch (err) {
                const msg = 'Could not write bluetooth: ' + err;
                this.showError(msg);
            }
        }
        if (this.sendArray.length > 0) {
            this.writeTimeoutId = setTimeout(this.writeBLE.bind(this), 0);
        } else {
            this.writeTimeoutId = null;
            this.isBusy = false;
        }
    }
}


export const pChair = new PChair();