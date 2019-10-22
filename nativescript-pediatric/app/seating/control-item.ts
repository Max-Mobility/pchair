export class ControlItem {
    public img: string = '';
    public name: string = '';
    public selected: boolean = false;

    constructor(name: string, img: string) {
        this.name = name;
        this.img = img;
        this.selected = false;
    }

    toString(): string {
        return `ControlItem: ${this.name} : ${this.img}`;
    }
}
