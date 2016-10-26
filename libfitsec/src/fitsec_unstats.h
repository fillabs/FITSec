/*********************************************************************
This file is a part of FItsSec project: Implementation of ETSI TS 103 097
Copyright (C) 2015  Denis Filatov (danya.filatov()gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed under GNU GPLv3 in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/gpl-3.0.txt>.
@license GPL-3.0+ <http://www.gnu.org/licenses/gpl-3.0.txt>

In particular cases this program can be distributed under other license
by simple request to the author. 
*********************************************************************/

#ifndef _MSC_VER
static unsigned short _un_stats_regions[] = {
	[1] = 0, /* World */
	[2] = 1, /* Africa */
	[4] = 34, /* Afghanistan */
	[5] = 419, /* South America */
	[8] = 39, /* Albania */
	[9] = 1, /* Oceania */
	[11] = 2, /* Western Africa */
	[12] = 15, /* Algeria */
	[13] = 419, /* Central America */
	[14] = 2, /* Eastern Africa */
	[15] = 2, /* Northern Africa */
	[16] = 61, /* American Samoa */
	[17] = 2, /* Middle Africa */
	[18] = 2, /* Southern Africa */
	[19] = 1, /* Americas */
	[20] = 39, /* Andorra */
	[21] = 19, /* Northern America */
	[24] = 17, /* Angola */
	[28] = 29, /* Antigua and Barbuda */
	[29] = 419, /* Caribbean */
	[30] = 142, /* Eastern Asia */
	[31] = 145, /* Azerbaijan */
	[32] = 5, /* Argentina */
	[34] = 142, /* Southern Asia */
	[35] = 142, /* South-Eastern Asia */
	[36] = 53, /* Australia */
	[39] = 150, /* Southern Europe */
	[40] = 155, /* Austria */
	[44] = 29, /* Bahamas */
	[48] = 145, /* Bahrain */
	[50] = 34, /* Bangladesh */
	[51] = 145, /* Armenia */
	[52] = 29, /* Barbados */
	[53] = 9, /* Australia and New Zealand */
	[54] = 9, /* Melanesia */
	[56] = 155, /* Belgium */
	[57] = 9, /* Micronesia */
	[60] = 21, /* Bermuda */
	[61] = 9, /* Polynesia */
	[64] = 34, /* Bhutan */
	[68] = 5, /* Bolivia (Plurinational State of) */
	[70] = 39, /* Bosnia and Herzegovina */
	[72] = 18, /* Botswana */
	[76] = 5, /* Brazil */
	[84] = 13, /* Belize */
	[90] = 54, /* Solomon Islands */
	[92] = 29, /* British Virgin Islands */
	[96] = 35, /* Brunei Darussalam */
	[100] = 151, /* Bulgaria */
	[104] = 35, /* Myanmar */
	[108] = 14, /* Burundi */
	[112] = 151, /* Belarus */
	[114] = 35, /* Cambodia */
	[115] = 17, /* Cameroon */
	[116] = 21, /* Canada */
	[132] = 11, /* Cabo Verde */
	[136] = 29, /* Cayman Islands */
	[140] = 17, /* Central African Republic */
	[142] = 1, /* Asia */
	[143] = 142, /* Central Asia */
	[144] = 34, /* Sri Lanka */
	[145] = 142, /* Western Asia */
	[148] = 17, /* Chad */
	[150] = 1, /* Europe */
	[151] = 150, /* Eastern Europe */
	[152] = 5, /* Chile */
	[154] = 150, /* Northern Europe */
	[155] = 150, /* Western Europe */
	[156] = 30, /* China */
	[170] = 5, /* Colombia */
	[174] = 14, /* Comoros */
	[175] = 14, /* Mayotte */
	[178] = 17, /* Congo */
	[180] = 17, /* Democratic Republic of the Congo */
	[184] = 61, /* Cook Islands */
	[188] = 13, /* Costa Rica */
	[191] = 39, /* Croatia */
	[192] = 29, /* Cuba */
	[196] = 145, /* Cyprus */
	[203] = 151, /* Czech Republic */
	[204] = 11, /* Benin */
	[208] = 154, /* Denmark */
	[212] = 29, /* Dominica */
	[214] = 29, /* Dominican Republic */
	[218] = 5, /* Ecuador */
	[222] = 13, /* El Salvador */
	[226] = 17, /* Equatorial Guinea */
	[231] = 14, /* Ethiopia */
	[232] = 14, /* Eritrea */
	[233] = 154, /* Estonia */
	[234] = 154, /* Faeroe Islands */
	[238] = 5, /* Falkland Islands (Malvinas) */
	[242] = 54, /* Fiji */
	[246] = 154, /* Finland */
	[248] = 154, /* Åland Islands */
	[250] = 155, /* France */
	[254] = 5, /* French Guiana */
	[258] = 61, /* French Polynesia */
	[262] = 14, /* Djibouti */
	[266] = 17, /* Gabon */
	[268] = 145, /* Georgia */
	[270] = 11, /* Gambia */
	[275] = 145, /* State of Palestine */
	[276] = 155, /* Germany */
	[288] = 11, /* Ghana */
	[292] = 39, /* Gibraltar */
	[296] = 57, /* Kiribati */
	[300] = 39, /* Greece */
	[304] = 21, /* Greenland */
	[308] = 29, /* Grenada */
	[312] = 29, /* Guadeloupe */
	[316] = 57, /* Guam */
	[320] = 13, /* Guatemala */
	[324] = 11, /* Guinea */
	[328] = 5, /* Guyana */
	[332] = 29, /* Haiti */
	[336] = 39, /* Holy See */
	[340] = 13, /* Honduras */
	[344] = 30, /* "China, Hong Kong Special Administrative Region" */
	[348] = 151, /* Hungary */
	[352] = 154, /* Iceland */
	[356] = 34, /* India */
	[360] = 35, /* Indonesia */
	[364] = 34, /* Iran (Islamic Republic of) */
	[368] = 145, /* Iraq */
	[372] = 154, /* Ireland */
	[376] = 145, /* Israel */
	[380] = 39, /* Italy */
	[384] = 11, /* Cote d'Ivoire */
	[388] = 29, /* Jamaica */
	[392] = 30, /* Japan */
	[398] = 143, /* Kazakhstan */
	[400] = 145, /* Jordan */
	[404] = 14, /* Kenya */
	[408] = 30, /* Democratic People's Republic of Korea */
	[410] = 30, /* Republic of Korea */
	[414] = 145, /* Kuwait */
	[417] = 143, /* Kyrgyzstan */
	[418] = 35, /* Lao People's Democratic Republic */
	[419] = 19, /* Latin America and the Caribbean */
	[422] = 145, /* Lebanon */
	[426] = 18, /* Lesotho */
	[428] = 154, /* Latvia */
	[430] = 11, /* Liberia */
	[434] = 15, /* Libya */
	[438] = 155, /* Liechtenstein */
	[440] = 154, /* Lithuania */
	[442] = 155, /* Luxembourg */
	[446] = 30, /* "China, Macao Special Administrative Region" */
	[450] = 14, /* Madagascar */
	[454] = 14, /* Malawi */
	[458] = 35, /* Malaysia */
	[462] = 34, /* Maldives */
	[466] = 11, /* Mali */
	[470] = 39, /* Malta */
	[474] = 29, /* Martinique */
	[478] = 11, /* Mauritania */
	[480] = 14, /* Mauritius */
	[484] = 13, /* Mexico */
	[492] = 155, /* Monaco */
	[496] = 30, /* Mongolia */
	[498] = 151, /* Republic of Moldova */
	[499] = 39, /* Montenegro */
	[500] = 29, /* Montserrat */
	[504] = 15, /* Morocco */
	[508] = 14, /* Mozambique */
	[512] = 145, /* Oman */
	[516] = 18, /* Namibia */
	[520] = 57, /* Nauru */
	[524] = 34, /* Nepal */
	[528] = 155, /* Netherlands */
	[531] = 29, /* Curaçao */
	[533] = 29, /* Aruba */
	[534] = 29, /* Sint Maarten (Dutch part) */
	[535] = 29, /* "Bonaire, Sint Eustatius and Saba" */
	[540] = 54, /* New Caledonia */
	[548] = 54, /* Vanuatu */
	[554] = 53, /* New Zealand */
	[558] = 13, /* Nicaragua */
	[562] = 11, /* Niger */
	[566] = 11, /* Nigeria */
	[570] = 61, /* Niue */
	[574] = 53, /* Norfolk Island */
	[578] = 154, /* Norway */
	[580] = 57, /* Northern Mariana Islands */
	[583] = 57, /* Micronesia (Federated States of) */
	[584] = 57, /* Marshall Islands */
	[585] = 57, /* Palau */
	[586] = 34, /* Pakistan */
	[591] = 13, /* Panama */
	[598] = 54, /* Papua New Guinea */
	[600] = 5, /* Paraguay */
	[604] = 5, /* Peru */
	[608] = 35, /* Philippines */
	[612] = 61, /* Pitcairn */
	[616] = 151, /* Poland */
	[620] = 39, /* Portugal */
	[624] = 11, /* Guinea-Bissau */
	[626] = 35, /* Timor-Leste */
	[630] = 29, /* Puerto Rico */
	[634] = 145, /* Qatar */
	[638] = 14, /* Réunion */
	[642] = 151, /* Romania */
	[643] = 151, /* Russian Federation */
	[646] = 14, /* Rwanda */
	[652] = 29, /* Saint-Barthélemy */
	[654] = 11, /* Saint Helena */
	[659] = 29, /* Saint Kitts and Nevis */
	[660] = 29, /* Anguilla */
	[662] = 29, /* Saint Lucia */
	[663] = 29, /* Saint Martin (French part) */
	[666] = 21, /* Saint Pierre and Miquelon */
	[670] = 29, /* Saint Vincent and the Grenadines */
	[674] = 39, /* San Marino */
	[678] = 17, /* Sao Tome and Principe */
	[680] = 154, /* Sark */
	[682] = 145, /* Saudi Arabia */
	[686] = 11, /* Senegal */
	[688] = 39, /* Serbia */
	[690] = 14, /* Seychelles */
	[694] = 11, /* Sierra Leone */
	[702] = 35, /* Singapore */
	[703] = 151, /* Slovakia */
	[704] = 35, /* Viet Nam */
	[705] = 39, /* Slovenia */
	[706] = 14, /* Somalia */
	[710] = 18, /* South Africa */
	[716] = 14, /* Zimbabwe */
	[724] = 39, /* Spain */
	[728] = 14, /* South Sudan */
	[729] = 15, /* Sudan */
	[732] = 15, /* Western Sahara */
	[740] = 5, /* Suriname */
	[744] = 154, /* Svalbard and Jan Mayen Islands */
	[748] = 18, /* Swaziland */
	[752] = 154, /* Sweden */
	[756] = 155, /* Switzerland */
	[760] = 145, /* Syrian Arab Republic */
	[762] = 143, /* Tajikistan */
	[764] = 35, /* Thailand */
	[768] = 11, /* Togo */
	[772] = 61, /* Tokelau */
	[776] = 61, /* Tonga */
	[780] = 29, /* Trinidad and Tobago */
	[784] = 145, /* United Arab Emirates */
	[788] = 15, /* Tunisia */
	[792] = 145, /* Turkey */
	[795] = 143, /* Turkmenistan */
	[796] = 29, /* Turks and Caicos Islands */
	[798] = 61, /* Tuvalu */
	[800] = 14, /* Uganda */
	[804] = 151, /* Ukraine */
	[807] = 39, /* The former Yugoslav Republic of Macedonia */
	[818] = 15, /* Egypt */
	[826] = 154, /* United Kingdom of Great Britain and Northern Ireland */
	[830] = 154, /* Channel Islands */
	[831] = 154, /* Guernsey */
	[832] = 154, /* Jersey */
	[833] = 154, /* Isle of Man */
	[834] = 14, /* United Republic of Tanzania */
	[840] = 21, /* United States of America */
	[850] = 29, /* United States Virgin Islands */
	[854] = 11, /* Burkina Faso */
	[858] = 5, /* Uruguay */
	[860] = 143, /* Uzbekistan */
	[862] = 5, /* Venezuela (Bolivarian Republic of) */
	[876] = 61, /* Wallis and Futuna Islands */
	[882] = 61, /* Samoa */
	[887] = 145, /* Yemen */
	[894] = 14, /* Zambia */
};
#else
static unsigned short _un_stats_regions[] = {
	0, 0, 1, 0, 34, 419, 0, 0,
	39, 1, 0, 2, 15, 419, 2, 2,
	61, 2, 2, 1, 39, 19, 0, 0,
	17, 0, 0, 0, 29, 419, 142, 145,
	5, 0, 142, 142, 53, 0, 0, 150,
	155, 0, 0, 0, 29, 0, 0, 0,
	145, 0, 34, 145, 29, 9, 9, 0,
	155, 9, 0, 0, 21, 9, 0, 0,
	34, 0, 0, 0, 5, 0, 39, 0,
	18, 0, 0, 0, 5, 0, 0, 0,
	0, 0, 0, 0, 13, 0, 0, 0,
	0, 0, 54, 0, 29, 0, 0, 0,
	35, 0, 0, 0, 151, 0, 0, 0,
	35, 0, 0, 0, 14, 0, 0, 0,
	151, 0, 35, 17, 21, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 11, 0, 0, 0,
	29, 0, 0, 0, 17, 0, 1, 142,
	34, 142, 0, 0, 17, 0, 1, 150,
	5, 0, 150, 150, 30, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 5, 0, 0, 0, 14, 14,
	0, 0, 17, 0, 17, 0, 0, 0,
	61, 0, 0, 0, 13, 0, 0, 39,
	29, 0, 0, 0, 145, 0, 0, 0,
	0, 0, 0, 151, 11, 0, 0, 0,
	154, 0, 0, 0, 29, 0, 29, 0,
	0, 0, 5, 0, 0, 0, 13, 0,
	0, 0, 17, 0, 0, 0, 0, 14,
	14, 154, 154, 0, 0, 0, 5, 0,
	0, 0, 54, 0, 0, 0, 154, 0,
	154, 0, 155, 0, 0, 0, 5, 0,
	0, 0, 61, 0, 0, 0, 14, 0,
	0, 0, 17, 0, 145, 0, 11, 0,
	0, 0, 0, 145, 155, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	11, 0, 0, 0, 39, 0, 0, 0,
	57, 0, 0, 0, 39, 0, 0, 0,
	21, 0, 0, 0, 29, 0, 0, 0,
	29, 0, 0, 0, 57, 0, 0, 0,
	13, 0, 0, 0, 11, 0, 0, 0,
	5, 0, 0, 0, 29, 0, 0, 0,
	39, 0, 0, 0, 13, 0, 0, 0,
	30, 0, 0, 0, 151, 0, 0, 0,
	154, 0, 0, 0, 34, 0, 0, 0,
	35, 0, 0, 0, 34, 0, 0, 0,
	145, 0, 0, 0, 154, 0, 0, 0,
	145, 0, 0, 0, 39, 0, 0, 0,
	11, 0, 0, 0, 29, 0, 0, 0,
	30, 0, 0, 0, 0, 0, 143, 0,
	145, 0, 0, 0, 14, 0, 0, 0,
	30, 0, 30, 0, 0, 0, 145, 0,
	0, 143, 35, 19, 0, 0, 145, 0,
	0, 0, 18, 0, 154, 0, 11, 0,
	0, 0, 15, 0, 0, 0, 155, 0,
	154, 0, 155, 0, 0, 0, 30, 0,
	0, 0, 14, 0, 0, 0, 14, 0,
	0, 0, 35, 0, 0, 0, 34, 0,
	0, 0, 11, 0, 0, 0, 39, 0,
	0, 0, 29, 0, 0, 0, 11, 0,
	14, 0, 0, 0, 13, 0, 0, 0,
	0, 0, 0, 0, 155, 0, 0, 0,
	30, 0, 151, 39, 29, 0, 0, 0,
	15, 0, 0, 0, 14, 0, 0, 0,
	145, 0, 0, 0, 18, 0, 0, 0,
	57, 0, 0, 0, 34, 0, 0, 0,
	155, 0, 0, 29, 0, 29, 29, 29,
	0, 0, 0, 0, 54, 0, 0, 0,
	0, 0, 0, 0, 54, 0, 0, 0,
	0, 0, 53, 0, 0, 0, 13, 0,
	0, 0, 11, 0, 0, 0, 11, 0,
	0, 0, 61, 0, 0, 0, 53, 0,
	0, 0, 154, 0, 57, 0, 0, 57,
	57, 57, 34, 0, 0, 0, 0, 13,
	0, 0, 0, 0, 0, 0, 54, 0,
	5, 0, 0, 0, 5, 0, 0, 0,
	35, 0, 0, 0, 61, 0, 0, 0,
	151, 0, 0, 0, 39, 0, 0, 0,
	11, 0, 35, 0, 0, 0, 29, 0,
	0, 0, 145, 0, 0, 0, 14, 0,
	0, 0, 151, 151, 0, 0, 14, 0,
	0, 0, 0, 0, 29, 0, 11, 0,
	0, 0, 0, 29, 29, 0, 29, 29,
	0, 0, 21, 0, 0, 0, 29, 0,
	0, 0, 39, 0, 0, 0, 17, 0,
	154, 0, 145, 0, 0, 0, 11, 0,
	39, 0, 14, 0, 0, 0, 11, 0,
	0, 0, 0, 0, 0, 0, 35, 151,
	35, 39, 14, 0, 0, 0, 18, 0,
	0, 0, 0, 0, 14, 0, 0, 0,
	0, 0, 0, 0, 39, 0, 0, 0,
	14, 15, 0, 0, 15, 0, 0, 0,
	0, 0, 0, 0, 5, 0, 0, 0,
	154, 0, 0, 0, 18, 0, 0, 0,
	154, 0, 0, 0, 155, 0, 0, 0,
	145, 0, 143, 0, 35, 0, 0, 0,
	11, 0, 0, 0, 61, 0, 0, 0,
	61, 0, 0, 0, 29, 0, 0, 0,
	145, 0, 0, 0, 15, 0, 0, 0,
	145, 0, 0, 143, 29, 0, 61, 0,
	14, 0, 0, 0, 151, 0, 0, 39,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 15, 0, 0, 0, 0, 0,
	0, 0, 154, 0, 0, 0, 154, 154,
	154, 154, 14, 0, 0, 0, 0, 0,
	21, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 29, 0, 0, 0, 11, 0,
	0, 0, 5, 0, 143, 0, 5, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 61, 0, 0, 0,
	0, 0, 61, 0, 0, 0, 0, 145,
	0, 0, 0, 0, 0, 0, 14,
};
#endif
