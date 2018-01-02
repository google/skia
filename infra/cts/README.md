Running CTS on Firebase Testlab
===============================

The run_testlab.go script uploads a given apk to Firebase Testlab and
runs them on the list of devices whitelisted in the script.
See the WHITELIST\_DEV\_IDS variable.

To run 'skqpapp.apk' on Testlab run the following command:

```
 $ go run run_testlab.go --logtostderr --service_account_file=service-account.json skqpapp.apk
```
