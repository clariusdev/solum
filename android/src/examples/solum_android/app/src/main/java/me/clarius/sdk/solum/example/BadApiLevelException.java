package me.clarius.sdk.solum.example;

public class BadApiLevelException extends Exception {
    public BadApiLevelException(int minimumLevel) {
        super("Must be at least API level " + minimumLevel);
    }
}
