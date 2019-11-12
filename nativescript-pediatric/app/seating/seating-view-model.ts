import { ObservableArray } from 'tns-core-modules/data/observable-array';
import { Observable } from 'tns-core-modules/data/observable';
import { Bluetooth, Characteristic, Device } from 'nativescript-bluetooth';
import { Toasty, ToastPosition } from 'nativescript-toasty';
import { ListViewEventData } from 'nativescript-ui-listview';
import { Prop } from '../obs-prop';
import { ControlItem } from './control-item';

export class SeatingViewModel extends Observable {
    @Prop() controlItems: ObservableArray<ControlItem> = new ObservableArray();
    @Prop() selectedControl: ControlItem = null;

    constructor() {
        super();
        this.controlItems.push([
            new ControlItem('Legs', '~/assets/images/leg.png'),
            new ControlItem('Back', '~/assets/images/back.png'),
            new ControlItem('Elevation', '~/assets/images/elev.png'),
            new ControlItem('Tilt', '~/assets/images/tilt.png'),
            new ControlItem('Standing Function', '~/assets/images/stand.png'),
            new ControlItem('Go-Kart Function', '~/assets/images/go.png')
        ]);
        this.selectedControl = this.controlItems.getItem(0);
        this.selectedControl.selected = true;
    }

    public async onControlSelected(event: ListViewEventData) {
        console.log('onControlTap()');
        this.controlItems.map(e => {
            e.selected = false;
        });
        this.selectedControl = this.controlItems.getItem(event.index);
        this.selectedControl.selected = true;
        console.log('control:', this.selectedControl.name);
    }
}
