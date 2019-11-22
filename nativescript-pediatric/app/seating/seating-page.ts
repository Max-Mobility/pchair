import { NavigatedData, Page } from 'tns-core-modules/ui/page';
import { SeatingViewModel } from './seating-view-model';
import { RadListView } from 'nativescript-ui-listview';

export function onNavigatingTo(args: NavigatedData) {
    const page = <Page>args.object;
    page.bindingContext = new SeatingViewModel();
    //console.log("seating page loaded.");
    // page.actionBarHidden = true;  
}

export function onPageLoaded(args: NavigatedData) {
    const page = <Page>args.object;
    const view:RadListView =page.getViewById("listview");
    //console.log("view: "+view);
    view.selectItemAt(0);  
}
