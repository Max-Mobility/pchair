import { Observable } from 'tns-core-modules/data/observable';
import { Bluetooth, Characteristic, Device } from 'nativescript-bluetooth';
import { Toasty, ToastPosition } from 'nativescript-toasty';
import { Prop } from '../obs-prop';

export class SeatingViewModel extends Observable {
    public static SERVICE_UUID: string = '000012ff-0000-1000-8000-00805f9b34fb';

    @Prop() peripheral: Device = null;
    @Prop() isBusy: boolean = false;

    private _bluetooth: Bluetooth;

    constructor() {
        super();
        this._bluetooth = new Bluetooth();
        this._bluetooth.debug = true;
    }

    public async scanAndConnect() {
        try {
            this.isBusy = true;
            console.log('scanAndConnect()');
            const gotPermissions = await this._bluetooth.requestCoarseLocationPermission();
            console.log('gotPermissions:', gotPermissions);
            const peripheral = await new Promise(async (resolve, reject) => {
                await this._bluetooth.startScanning({
                    serviceUUIDs: [SeatingViewModel.SERVICE_UUID],
                    seconds: 4,
                    onDiscovered: async (peripheral: Device) => {
                        console.log('onDiscovered:', peripheral);
                        // stop scanning
                        await this._bluetooth.stopScanning();
                        // resolve
                        resolve(peripheral);
                    }
                });
                // if we've gotten to here then we've timed out without finding
                // any devices
                reject('Timeout searching for device');
            }) as Device;
            // we have a peripheral - so connect to it
            console.log('peripheral:', peripheral);
            await new Promise(async (resolve, reject) => {
                this._bluetooth.connect({
                    UUID: peripheral.address,
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
            // if we've gotten here then we're connected
            console.log('connected');
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

    public async onBluetoothTap() {
        console.log('onBluetoothTap()');
        await this.scanAndConnect();
    }

    private async stopNotifyCharacteristics(
        characteristics: Array<Characteristic>
    ): Promise<any> {
        try {
            for (const c of characteristics) {
                await this._bluetooth.stopNotifying({
                    peripheralUUID: this.peripheral.address,
                    serviceUUID: SeatingViewModel.SERVICE_UUID,
                    characteristicUUID: c.UUID
                });
            }
        } catch (err) {
            console.error('could not stop notifying', err);
        }
    }

    private async startNotifyCharacteristics(
        characteristics: Array<Characteristic>
    ): Promise<any> {
        try {
            for (const c of characteristics) {
                await this._bluetooth.startNotifying({
                    peripheralUUID: this.peripheral.address,
                    serviceUUID: SeatingViewModel.SERVICE_UUID,
                    characteristicUUID: c.UUID,
                    onNotify: this.onNotify.bind(this)
                });
            }
        } catch (err) {
            console.error('could not start notifying', err);
        }
    }

    private async onConnected(peripheral: Device) {
        this.peripheral = peripheral;
        this.peripheral.services.forEach(async (service) => {
            await this.startNotifyCharacteristics(service.characteristics);
        });
    }

    private async onDisconnected(peripheral: Device) {
        this.peripheral = peripheral;
    }

    private async onNotify(args: any) {
        console.log('onNotify:', args);
    }
}
