# sample script for retrieving the scanner certificates through the clarius rest api
# should be relatively simple to convert to curl, java, or other frameworks

import requests
import json

token="your_token_here"
url = "https://cloud.clarius.com/api/public/v0/devices/oem/?format=json"
hdr = {'Authorization' : 'OEM-API-Key {}'.format(token)}

# make the request
resp = requests.get(url, headers=hdr)

authenticated = []

# check for valid response
if resp.status_code == 200:
    js = resp.json()
    probes = js["results"]

    for probe in probes:
        # ensure we have a valid certificate
        if "crt" in probe:
            device = probe["device"]
            authenticated.append({ device["serial"], probe["crt"] })

    # display all authenticated probes
    for auth in authenticated:
        print(auth)
else:
    print("error making request: ", resp)
