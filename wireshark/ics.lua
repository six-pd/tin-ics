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

	local request = buffer(0,1):string()
	local req_name = get_req_name(request)

	subtree:add_le(req, buffer(0,1)):append_text(" - " .. req_name)
	subtree:add_le(num, buffer(1,1))
	subtree:add_le(message, buffer(3,(length - 4)))
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(45456, ics_protocol)

function get_req_name(request)
	local req_name = "UNKNOWN"

	if request == '0' then req_name = "CONNECT"
	elseif request == '1' then req_name = "DISCONNECT"	
	elseif request == '2' then req_name = "SEND FILE"
	elseif request == '3' then req_name = "RECEIVE FILE"
	elseif request == '4' then req_name = "CLIENT LIST"
	elseif request == '9' then req_name = "ERROR" end

	return req_name
end
