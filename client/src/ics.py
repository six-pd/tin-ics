#!/usr/bin/python3
import pexpect
import sys

def communicate():
    while client.isalive():
        client.expect('>>')
        print(client.before.decode('ASCII'))
        client.sendline(input('>'))

client = pexpect.spawn('./client.out')
print("Welcome to the ICS Client Test Interface")

communicate()

print("Interface shutting down")
