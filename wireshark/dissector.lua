ics_protocol = Proto("ICS", "ICS Communication Protocol")

id =

id_num =

message =

ics_protocol.fields = {id, id_num, message}

function ics_protocol.dissector(buffer, pinfo, tree)
	length = buffer:len()
	if length == 0 then return end

	pinfo.cols.protocol = ics_protocol.name

	local subtree = tree:add(ics_protocol, buffer(), "ICS Communication Data")

	subtree:add_le(id, buffer(0,1))
	subtree:add_le(id_num, buffer(1,1))
	subtree:add_le(message, buffer(3,(length - 1)))
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(45456, ics_protocol)

