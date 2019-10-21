import { NavigatedData, Page } from "tns-core-modules/ui/page";
import { SeatingViewModel } from "./seating-view-model";

export function onNavigatingTo(args: NavigatedData) {
    const page = <Page>args.object;
    page.bindingContext = new SeatingViewModel();
}
