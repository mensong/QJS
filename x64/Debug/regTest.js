
Reg.createKey('HKEY_CURRENT_USER\\SOFTWARE\\我爱你123', false);
Reg.WriteStringValue('HKEY_CURRENT_USER\\SOFTWARE\\我爱你123', 'test', '我爱你123', false);
var val = Reg.ReadStringValue('HKEY_CURRENT_USER\\SOFTWARE\\我爱你123', 'test', false);
alert(val);
Reg.deleteValue('HKEY_CURRENT_USER\\SOFTWARE\\我爱你123', 'test', false);
Reg.deleteTree('HKEY_CURRENT_USER\\SOFTWARE\\我爱你123', false);

var subKeys = Reg.EnumSubKeys('HKEY_LOCAL_MACHINE\\SOFTWARE');
for(var i=0; i<subKeys.length; ++i)
{
	print(subKeys[i] + '\n');
}

var valueNameKeys = Reg.EnumValueNameKeys('HKEY_LOCAL_MACHINE\\SOFTWARE\\DefaultUserEnvironment');
for(var i=0; i<valueNameKeys.length; ++i)
{
	print(valueNameKeys[i] + '\n');
}
