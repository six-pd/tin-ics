ics_protocol = Proto("ICS", "ICS Communication Protocol")

req = ProtoField.new("Request", "ics.request", ftypes.CHAR)

num = ProtoField.new("Request Sequence Number", "ics.seqnum", ftypes.CHAR)

message = ProtoField.new("Message", "ics.message", ftypes.STRING)

ics_protocol.fields = {req, num, message}

function ics_protocol.dissector(buffer, pinfo, tree)
	length = buffer:len()
	if length == 0 then return end

	pinfo.cols.protocol = ics_protocol.name

	local subtree = tree:add(ics_protocol, buffer(), "ICS Communication Data")

	subtree:add_le(req, buffer(0,1))
	subtree:add_le(num, buffer(1,1))
	subtree:add_le(message, buffer(3,(length - 4)))
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(45456, ics_protocol)

