import{AndroidSensors,AndroidSensorListener,SensorDelay} from 'nativescript-android-sensors'
import { DeviceBase } from './device-base';

export class Sensor extends DeviceBase{
public _sensor:AndroidSensors;
public accelerometerSensor:android.hardware.Sensor;
public sensorListener;
public static sensor_gravity_event = 'sensor_gravity_event';
    constructor(){
        super();
        this._sensor= new AndroidSensors(true);

        this.sensorListener = new AndroidSensorListener({
            onAccuracyChanged: (
                sensor: android.hardware.Sensor,
                accuracy: number
              ) => {
                //console.log('accuracy', accuracy);
            },
            onSensorChanged:(result:string)=>{
                const parsedData = JSON.parse(result);
                // const rawSensorData = parsedData.data;
                // const sensor = parsedData.sensor;
                // const time = parsedData.time;
                if (parsedData.s ="9") // gravity = 9
                {
                    const x : number = parseFloat(parsedData.d.x);
                    const y : number = parseFloat(parsedData.d.y);
                    const z : number = parseFloat(parsedData.d.z);
                    this.sendEvent(Sensor.sensor_gravity_event,{
                        x:x,
                        y:y,
                        z:z
                    });
                    // console.log("get gravity data: "+result);
                }
            }
        })
        this._sensor.setListener(this.sensorListener);
        this.stopAcc();
        // this.startAcc();

    };
    public startAcc(){
        console.log("start Acc.");
        this.accelerometerSensor =this._sensor.startSensor(android.hardware.Sensor.TYPE_GRAVITY, SensorDelay.NORMAL);
    }

    public stopAcc()
    {
        console.log("Stop Acc.");
        this._sensor.stopSensor(this.accelerometerSensor);
    }
}