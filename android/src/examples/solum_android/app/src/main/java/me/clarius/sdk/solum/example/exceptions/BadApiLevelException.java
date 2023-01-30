package me.clarius.sdk.solum.example.exceptions;

public class BadApiLevelException extends Exception {
    public BadApiLevelException(int minimumLevel) {
        super("Must be at least API level " + minimumLevel);
    }
}
