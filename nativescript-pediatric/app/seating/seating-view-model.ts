import { Observable } from 'tns-core-modules/data/observable';
import * as bluetooth from 'nativescript-bluetooth';

export class SeatingViewModel extends Observable {
    public static SERVICE_UUID: string = '000012ff-0000-1000-8000-00805f9b34fb';

    public peripheral: bluetooth.Peripheral = null;

    constructor() {
        super();

    }

    public async scanAndConnect() {
        try {
            const peripheral = await new Promise(async (resolve, reject) => {
                await bluetooth.startScanning({
                    serviceUUIDs: [SeatingViewModel.SERVICE_UUID],
                    seconds: 4,
                    skipPermissionCheck: false,
                    onDiscovered: async function(peripheral: bluetooth.Peripheral) {
                        // stop scanning
                        await bluetooth.stopScanning();
                        // resolve
                        resolve(peripheral);
                    }
                });
                // if we've gotten to here then we've timed out without finding
                // any devices
                reject('timeout');
            }) as bluetooth.Peripheral;
            // we have a peripheral - so connect to it
            await new Promise(async (resolve, reject) => {
                bluetooth.connect({
                    UUID: peripheral.UUID,
                    onConnected: function(peripheral: bluetooth.Peripheral) {
                        this.onConnected(peripheral);
                        resolve(peripheral);
                    },
                    onDisconnected: function(peripheral: bluetooth.Peripheral) {
                        this.onDisconnected(peripheral);
                        reject('Could not connect to' + peripheral);
                    }
                });
            });
            // if we've gotten here then we're connected
        } catch (err) {
        }
    }

    private async onConnected(peripheral: bluetooth.Peripheral) {
        this.peripheral = peripheral;
    }

    private async onDisconnected(peripheral: bluetooth.Peripheral) {
        this.peripheral = peripheral;
    }
}
