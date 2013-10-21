#include "cjdc.h"

char P1[] = \
"d4:morei1e5:peersld7:bytesIni171823479e8:bytesOuti202386895e10:duplicatesi0e10:isIncomingi1e4:lasti1381940490142e11:lostPacketsi178e9:publicKey54:dk6ntf932hkpx6vrnm0m18shvpd3wr3ptlf7y6yu13nlkk9tdxs0.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.001d4:user8:pentagoned7:bytesIni788480907e8:bytesOuti762277408e10:duplicatesi0e10:isIncomingi1e4:lasti1381940490314e11:lostPacketsi224e9:publicKey54:8krh1n5k9yng2h6kdz7gu381kfys4yjq1kv3x1xb0vlnywmqk210.k18:receivedOutOfRangei4e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.001b4:user7:hal2001ed7:bytesIni1685226798e8:bytesOuti1696313454e10:duplicatesi0e10:isIncomingi1e4:lasti1381940490418e11:lostPacketsi756e9:publicKey54:5pv9dthnlkyyn2uh4mzfrnp3gwdzh57n9sq4ljp00511zmvmmzd0.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.00194:user6:hardaxed7:bytesIni809907463e8:bytesOuti802560023e10:duplicatesi0e10:isIncomingi0e4:lasti1381940490350e11:lostPacketsi36e9:publicKey54:y41hb628jpbq9h3v98ksb5q55sn9v8gmc65f20dx190nv2xgz1f0.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.0017ed7:bytesIni562568690e8:bytesOuti593290685e10:duplicatesi0e10:isIncomingi0e4:lasti1381940490315e11:lostPacketsi12e9:publicKey54:t21512czugsv0fzw4z0pym4lphj060kggduqkw64mqvmlqrws8u0.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.0015ed7:bytesIni415469524e8:bytesOuti490117620e10:duplicatesi0e10:isIncomingi0e4:lasti1381940490366e11:lostPacketsi8114e9:publicKey54:ll5t0k0y957lj2s20bqzdu1qtgs7j316ss5p5c8w7xhxxs5zj880.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.0013ee5:totali10e4:txid10:BBCDEABCDEe";
char P2[] = \
"d5:peersld7:bytesIni12247938e8:bytesOuti14306976e10:duplicatesi0e10:isIncomingi1e4:lasti1381940488882e11:lostPacketsi4e9:publicKey54:bt9cb2yuk7mgsctcp7h3gu376978m1tdb50j8r9wv5vrph5w9020.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.009e4:user3:pi2ed7:bytesIni45087615e8:bytesOuti49004406e10:duplicatesi0e10:isIncomingi1e4:lasti1381940490404e11:lostPacketsi57e9:publicKey54:mm3fccgskbsjf22tvfvq70uvzd7x55gjsn9qgkr3q6g8drmy1580.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.00a64:user12:__name12____ed7:bytesIni314245626e8:bytesOuti294564663e10:duplicatesi0e10:isIncomingi1e4:lasti1381940490326e11:lostPacketsi183e9:publicKey54:9fnm32z23dxm4j7mn1sm9l46kbjywhltqcxubc6rrkz0fm6w5up0.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.00a24:user5:franned7:bytesIni614308680e8:bytesOuti562148717e10:duplicatesi0e10:isIncomingi1e4:lasti1381940490396e11:lostPacketsi185e9:publicKey54:1w3sj6g38c6vm5cn3d42dvwn3bc7mxndjtxw9dfj0s3n3jrbpwy0.k18:receivedOutOfRangei0e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.001f4:user5:michaed7:bytesIni1052862870e8:bytesOuti1166057113e10:duplicatesi0e10:isIncomingi0e4:lasti1382313757709e11:lostPacketsi360e9:publicKey54:crpnq31d8gskz5tfb5f4u3bv4qszfmkhu0p3cvwb9mfhvzz6z730.k18:receivedOutOfRangei4e5:state11:ESTABLISHED11:switchLabel19:0000.0000.0000.0017ee5:totali11e4:txid10:BBCDEABCDEe";
char P3[] = \
"d5:peersle5:totali10e4:txid10:BBCDEABCDEeerror";

int main()
{
	int n;

	if ((n = peers(P1, sizeof(P1) - 1)) != 1)
		return 1;

	if ((n = peers(P2, sizeof(P2) - 1)) != 0)
		return 2;

	if ((n = peers(P3, sizeof(P3) - 1)) != 0)
		return 3;

	return n;
}
