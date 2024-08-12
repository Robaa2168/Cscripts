from pesapy.b2c import B2C

resp = B2C.process_transaction(
    command_id="BusinessPayment", amount=500, phone_number="254798530725",
    remarks="Requested on Tuesday", occassion="issue closed"
)
print(resp)