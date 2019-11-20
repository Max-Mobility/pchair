import { Observable } from 'tns-core-modules/data/observable';

export class DeviceBase extends Observable {
    public sendEvent(eventName: string, data?: any, msg?: string) {
        this.notify({
          eventName,
          object: this,
          data,
          message: msg
        });
    }
}