package com.example.ble;

import java.util.HashMap;

public class SampleGattAttributes {
    private static HashMap<String, String> attributes = new HashMap();

    //TODO: NEED TO LOOKUP AND CHANGE

    public static String MEASURE = "000012ff-0000-1000-8000-00805f9b34fb";
    public static String POSITION_1 = "0000ff01-0000-1000-8000-00805f9b34fb";
    public static String POSITION_2 = "0000ff02-0000-1000-8000-00805f9b34fb";
    public static String POSITION_3 = "0000ff03-0000-1000-8000-00805f9b34fb";
    public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";

    static {

        //Sample Services.
        attributes.put("00001800-0000-1000-8000-00805f9b34fb", "Generic Access Service");
        attributes.put("00001801-0000-1000-8000-00805f9b34fb", "Generic Attribute Service");
        attributes.put("000012ff-0000-1000-8000-00805f9b34fb", "User Design Service");

        // Sample Characteristics.
        attributes.put(POSITION_1, "Position 1");
        attributes.put(POSITION_2, "Position 2");
        attributes.put(POSITION_3, "Position 3");
    }

    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }

}
