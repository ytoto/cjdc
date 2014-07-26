# cjdc, another cjdns admin tool
This is a small utility, written in C language.

## usage

	cjdc [command]

	commands:
	dump		dumps the routing table
	peers		lists connected nodes
	<pub.k>		public key to ipv6

## sample output

	# cjdc -v peers
	ip6                                     switch               in   out state dup  los  oor user
	fc8d:7f0e:1b26:5270:3498:cd1f:c49f:e762 0000.0000.0000.001b 751M 726M INVA   0  224    4  hal2001
	fca7:0520:11c0:11df:5011:316e:abc1:3e37 0000.0000.0000.0019 1.5G 1.5G ESTA   0  756    0  hardax
	fc05:1349:9fff:8e6b:c637:b9b6:4cd5:53e8 0000.0000.0000.0017 772M 765M HAND   0   36    0
	fcab:522d:807e:1841:8069:6cc1:f0c6:8632 0000.0000.0000.0015 536M 565M UNAU   0   12    0
	fc82:5dcf:9465:1e97:8807:fedb:c075:c7f3 0000.0000.0000.0013 396M 467M UNRE   0  7.9K   0
	fc44:bc05:661c:2614:c9c6:9e68:5cca:a5f6 0000.0000.0000.00a2 299M 280M ESTA   0  183    0  frann
	fc65:5586:2ece:97bd:c6d2:3ee6:ba63:bbe9 0000.0000.0000.001f 585M 536M ESTA   0  185    0  micha
	fc73:cc4b:569f:eb04:4583:47e8:b67d:9de6 0000.0000.0000.0017 999M 1.0G ESTA   0  360    4

### header description

	ip6	cjdns IPv6 address
	switch	switchLabel
	in	bytesIn
	out	bytesOut
	state	link state
	dup	duplicate packets
	los	lostPackets
	oor	receivedOutOfRange
	user	the user field of the password entry in cjdroute.conf

#### example cjdroute.conf password with user field
	"authorizedPasswords":
	[
	    {"password": "................", "user": "micha"},

## internals
cjdc uses the [cjdns admin interface](https://wiki.projectmeshnet.org/Admin_Interface), see also the
[cjdns admin API](https://github.com/cjdelisle/cjdns/blob/master/admin/README.md).
