import { NavigatedData, Page } from "tns-core-modules/ui/page";

import { DrivingViewModel } from "./driving-view-model";

export function onNavigatingTo(args: NavigatedData) {
    const page = <Page>args.object;
    const vm = new DrivingViewModel();

    page.bindingContext = vm;
    //console.log("driving page loaded.");
    //vm.onPageLoaded(args);
    // page.actionBarHidden = true;
}
