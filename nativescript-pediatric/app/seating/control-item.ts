import { Observable } from 'tns-core-modules/data/observable';
import { ImageSource, fromFile, fromResource, fromBase64 } from "tns-core-modules/image-source";
import { Prop } from '../obs-prop';

export class ControlItem extends Observable {
    @Prop() img: string = '';
    @Prop() name: string = '';
    @Prop() selected: boolean = false;

    constructor(name: string, img: string) {
        super();
        this.name = name;
        this.img = img;
        this.selected = false;
    }
}
