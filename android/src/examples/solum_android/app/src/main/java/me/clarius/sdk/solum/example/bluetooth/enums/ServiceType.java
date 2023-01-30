package me.clarius.sdk.solum.example.bluetooth.enums;

import java.util.UUID;

/**
 * Represents the type of Clarius custom BLE service.
 *
 * @see <a href="https://github.com/clariusdev/solum/blob/main/specifications.md#connectivity">Types of BLE services offered by Clarius scanners</a>
 */
public enum ServiceType {
    /**
     * <h2>Custom service built by Clarius to read and manage the Wi-Fi network once the scanner is powered up and ready.</h2>
     *
     * <h3>Wi-Fi Published Characteristic</h3>
     * <p>
     *     Once ready, the current Wi-Fi network information will be published through the Wi-Fi Published characteristic,
     *     and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification will take place
     *     when the information has changed. If the service reads "N/A", it typically means the probe has not finished booting to a ready state.
     * </p>
     * <p>
     *     To subscribe to the Wi-Fi Published characteristic, one can write 0100 to the characteristic's Client
     *     Characteristic Configuration Descriptor (CCCD), allowing the probe to send out notifications to the connected program
     * </p>
     *
     * <br>
     * <h3>Wi-Fi Request Characteristic</h3>
     * <p>
     *     To change network configurations, the Wi-Fi Request characteristic can be written to.
     *     Note that the probe must be in a ready state before the request will have any effect.
     * </p>
     *
     * @see <a href="https://github.com/clariusdev/solum/blob/main/specifications.md#wi-fi-information-service">Detailed Wi-Fi Service Information</a>
     */
    WifiInformationService
    (
            UUID.fromString("F9EB3FAE-947A-4E5B-AB7C-C799E91ED780"),
            UUID.fromString("F9EB3FAE-947A-4E5B-AB7C-C799E91ED781"),
            UUID.fromString("F9EB3FAE-947A-4E5B-AB7C-C799E91ED782")
    ),
    /**
     * <h2>Custom service built by Clarius to read and manage the power status of the scanner.</h2>
     *
     * <h3>Power Published Characteristic</h3>
     * <p>
     *     Once ready, the device powered status will be published through the Power Published characteristic,
     *     and can be read at any point after a BLE connection, as well as subscribed to, and thus a notification
     *     will take place when the information has changed. The read and notifications should always be 1 byte, with 0 denoting an off state,
     *     and 1 denoting an on state.
     * </p>
     * <p>
     *     To subscribe to the Power Published characteristic, one can write 0100 to the characteristic's Client
     *     Characteristic Configuration Descriptor (CCCD), allowing the probe to send out notifications to the connected program.
     * </p>
     * <br>
     * <h3>Power Request Characteristic</h3>
     * <p>
     *     To power on or off the device, the Power Request characteristic can be written to. Writing 0x00 to the characteristic
     *     will power the device off, and writing 0x01 will power the device on.
     * </p>
     */
    PowerService
    (
            UUID.fromString("8C853B6A-2297-44C1-8277-73627C8D2ABC"),
            UUID.fromString("8C853B6A-2297-44C1-8277-73627C8D2ABD"),
            UUID.fromString("8C853B6A-2297-44C1-8277-73627C8D2ABE")
    );

    /**
     * The UUID of this service.
     */
    public final UUID serviceUuid;
    /**
     * The UUID of the characteristic for reading the value (read/subscribe).
     */
    public final UUID infoCharacteristicUuid;
    /**
     * The UUID of the characteristic for writing requests.
     */
    public final UUID requestCharacteristicUuid;

    private ServiceType
            (
                    UUID serviceUuid,
                    UUID infoCharacteristicUuid,
                    UUID requestCharacteristicUuid
            )
    {
        this.serviceUuid = serviceUuid;
        this.infoCharacteristicUuid = infoCharacteristicUuid;
        this.requestCharacteristicUuid = requestCharacteristicUuid;
    }
}
