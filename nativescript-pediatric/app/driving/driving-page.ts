import { NavigatedData, Page } from "tns-core-modules/ui/page";

import { DrivingViewModel } from "./driving-view-model";

const vm = new DrivingViewModel();

export function onNavigatingTo(args: NavigatedData) {
    const page = <Page>args.object;
    page.bindingContext = vm;
    //console.log("driving page loaded.");
    //vm.onPageLoaded(args);
    // page.actionBarHidden = true;
}

export function onPageLoaded(args: NavigatedData) {
    vm.sendMaxSpeed();
}
