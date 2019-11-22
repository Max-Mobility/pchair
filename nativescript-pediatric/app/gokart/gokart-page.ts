import { NavigatedData, Page } from "tns-core-modules/ui/page";

import { GokartViewModel } from "./gokart-view-model";

const vm = new GokartViewModel();
export function onNavigatingTo(args: NavigatedData) {
    const page = <Page>args.object;
    

    page.bindingContext = vm;
    //console.log("gokart page loaded.");
    //vm.onPageLoaded(args);
    // page.actionBarHidden = true;
}
export function onPageLoaded(args: NavigatedData) {
    

    vm.clearCalibration();
    //console.log("gokart page loaded.");
    //vm.onPageLoaded(args);
    // page.actionBarHidden = true;
}

export function onPageUnloaded(args: NavigatedData) {
    //console.log("onPageUnloaded gokart.");
    vm.clearCalibration();
    vm.stopSensor();
}