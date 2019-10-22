import { Observable } from "tns-core-modules/data/observable";
import { Prop } from '../obs-prop';

export class DrivingViewModel extends Observable {
    @Prop() forwardSpeed: number = 75;
    @Prop() turningSpeed: number = 50;
    @Prop() acceleration: number = 30;

    constructor() {
        super();
    }
}
