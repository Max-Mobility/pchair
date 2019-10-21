import { NavigatedData, Page } from "tns-core-modules/ui/page";

import { DrivingViewModel } from "./driving-view-model";

export function onNavigatingTo(args: NavigatedData) {
    const page = <Page>args.object;
    page.bindingContext = new DrivingViewModel();
    page.actionBarHidden = true;
}
