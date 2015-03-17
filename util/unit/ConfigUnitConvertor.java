import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*
 * This program will read a libconfig cfg file, will read the definition in value_types.ini and will convert all the units to SI.
 * 
 *
 */

enum SystemType
{
	SI, 	// International system 
	AU;  	// Atomic unit
}
enum ValueType
{  
	MASS,
	LENGTH,
	CHARGE,
	ENERGY,
	TIME,
	FREQUENCY,
	PULSATION,
	SPEED,
	FORCE,
	ELECTRIC_FIELD,
	ELECTRIC_POTENTIAL,
	MOMENTUM,
	MAGNETIC_FIELD,
	ANGLE,
	PERCENTUAL,
	PURE_FLOAT,
	PURE_INT,
	IGNORE,
	IGNORE_START,
	IGNORE_END,
}

class PrefixType
{
	String name;
	String symbol;
	int	   power;
	
	
	public PrefixType(String name, String symbol, int power)
	{
		this.name = name;
		this.symbol = symbol;
		this.power = power;
	}
	
	@Override
	public String toString()
	{
		return String.format("%s (%s) 10^%d", symbol, name, power);
	}
	
}

class UnitType
{
	String 				symbol;
	Double 				siValue;
	String 				name;
	List<SystemType>	systems = new ArrayList<>();
	String				note;
	
	@Override
	public String toString()
	{
		if (name != null && !name.isEmpty())
			return String.format("%s \t (%s)", symbol, name);
		else
			return String.format("%s", symbol);
	}
	
}


public class ConfigUnitConvertor
{
	private static Pattern pattern1;
	private static Pattern pattern2;
	private static Pattern pattern3;
	static
	{
		pattern1 = Pattern.compile("^\\s*(\\w+)\\s*=.*");

		String expRegex = "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";
		String muRegex  = "[Α-Ωα-ωa-zA-Z\\_\\*\\/\\%\\°]";
		pattern2 	= Pattern.compile("^\\s*(\\w+)\\s*=\\s*(" + expRegex + ")\\s*((" + muRegex + ")*)\\s*$");
		
		
		pattern3 = Pattern.compile("^\\s*\\#\\s*unit\\_type\\s*:\\s*\\[([\\s\\w]+)\\].*$");
	}

	
	private static List<PrefixType> loadPrefixes()
	{
		List<PrefixType> prefixes = new ArrayList<>();
		prefixes.add(new PrefixType ("yotta", 	"Y",	 +24 ));
		prefixes.add(new PrefixType ("zetta", 	"Z",	 +21 ));
		prefixes.add(new PrefixType ("exa", 	"E",	 +18 ));
		prefixes.add(new PrefixType ("peta", 	"P",	 +15 ));
		prefixes.add(new PrefixType ("tera", 	"T",	 +12 ));
		prefixes.add(new PrefixType ("giga", 	"G",	 +9  ));
		prefixes.add(new PrefixType ("mega", 	"M",	 +6  ));
		prefixes.add(new PrefixType ("kilo", 	"k",	 +3  ));
		prefixes.add(new PrefixType ("centi", 	"c",	 -2  ));
		prefixes.add(new PrefixType ("milli", 	"m",	 -3  ));
		prefixes.add(new PrefixType ("micro", 	"μ",	 -6  ));
		prefixes.add(new PrefixType ("nano", 	"n",	 -9  ));
		prefixes.add(new PrefixType ("pico", 	"p",	 -12 ));
		prefixes.add(new PrefixType ("femto", 	"f",	 -15 ));
		prefixes.add(new PrefixType ("atto", 	"a",	 -18 ));
		prefixes.add(new PrefixType ("zepto", 	"z",	 -21 ));
		prefixes.add(new PrefixType ("yocto", 	"y",	 -24 ));
		
		return prefixes;
		
	}
	private static Map<ValueType, List<UnitType>> loadConversions(String filename)
	{
		Map<ValueType, List<UnitType>> types = new HashMap<ValueType, List<UnitType>>();
		
		for (ValueType type: ValueType.values())
		{
			types.put(type, new ArrayList<UnitType>()); 
		}
		
		String[] lines = loadFileContent(filename);
		
		int i = 0;
		for (String line: lines)
		{
			i++;
			if (line.trim().length() > 0 && !line.trim().startsWith("#"))
			{
				String[] fields = line.split("\\,");
				
				if (fields.length >= 5)
				{
					UnitType unitType = new UnitType();
					
					ValueType valueType;
					
					try
					{	
						valueType = ValueType.valueOf(fields[0].trim());
					} catch (IllegalArgumentException e)
					{
						System.out.printf("ERROR in file '%s' at line %d: the 1st field must be one of %s\n", filename, i, getPossibleValues(ValueType.values()));
						System.exit(-1);
						return null;
					}
					
					unitType.symbol = fields[1].trim();
					
					if (unitType.symbol.length() == 0)
					{
						System.out.printf("ERROR in file '%s' at line %d: the 2nd field is mandatory", filename, i);
						System.exit(-1);
						return null;
					}
					
					try
					{
						unitType.siValue = Double.parseDouble(fields[2].trim());
					}
					catch (NumberFormatException e)
					{
						System.out.printf("ERROR in file '%s' at line %d: the 3rd field is not a valid number", filename, i);
						System.exit(-1);
						return null;
					}
					
					unitType.name = fields[3].trim();
					
					if (unitType.name.length() == 0)
					{
						System.out.printf("ERROR in file '%s' at line %d: the 4th field is mandatory", filename, i);
						System.exit(-1);
						return null;
					}
					
					
					if (!fields[4].trim().isEmpty())
					{
						String[] systemsStr = fields[4].toUpperCase().replaceAll("\\s", "").split("\\|");
						
						for (String systemStr: systemsStr)
						{
							try
							{
								unitType.systems.add(SystemType.valueOf(systemStr));
							} catch (IllegalArgumentException e)
							{
								System.out.printf("ERROR in file '%s' at line %d: the 5th field must be one of %s\n", filename, i, getPossibleValues(SystemType.values()));
								System.exit(-1);
								return null;
							}
						}
					}	
					
					if (fields.length >= 6)
						unitType.note = fields[5].trim();
					
					types.get(valueType).add(unitType);
					
				}
				else
				{
					System.out.printf("ERROR in file '%s' at line %d: the line is not composed by 6 fields seprated by comma.\n", filename, i);
					System.exit(-1);
					return null;
				}
				
			}
		}
		
		
		// Sorting the units in decrescing symbol length
		for (ValueType typeKey: types.keySet())
		{
			Collections.sort(types.get(typeKey), new Comparator<UnitType>()
			{
				@Override
				public int compare(UnitType o1, UnitType o2)
				{
					return -Integer.compare(o1.symbol.length(), o2.symbol.length());
				}
				
			});
			
		}
		
		
		
		return types;
	}

	private static String[] loadFileContent(String filename)
	{
		// Reading the CSV with old style programming
		// We could use a library but we want to keep this program as simple as possible
		
		
		// Just to be clear: I am not so proud of this code :)
		File file = new File(filename);
		
		String content;
		
		try(FileInputStream fis = new FileInputStream(file))
		{
			byte[] data = new byte[(int) file.length()];
		
			fis.read(data);
			 content = new String(data, "UTF-8");
		}
		catch (FileNotFoundException e)
		{
			System.out.printf("Unable to open file '%s'.\n", filename);
			System.exit(1);
			return null;
		}
		catch (IOException e)
		{
			throw new RuntimeException(e);
		}
		
		// Parsing the file content in a very unperformant way
		
		String[] lines = content.split("\\r?\\n");
		return lines;
	}
	
	private static void saveFileContent(String filename, ArrayList<String> content)
	{

		File file = new File(filename);
		
		try(FileOutputStream fos = new FileOutputStream(file))
		{
			for (String line: content)
			{
				fos.write(line.getBytes());
				fos.write("\n".getBytes());
			}
		}
		catch (IOException e)
		{
			throw new RuntimeException(e);
		}
	}

	private static ValueType getTemplate(String line, String filename, int i)
	{
		
		Matcher matcher3 = pattern3.matcher(line);
		
		
		if (matcher3.matches())
		{
			String s1 = matcher3.group(1);
			
			String s2 = s1.toUpperCase().trim().replaceAll("\\s+", "_").toUpperCase();
				
			
			try
			{
				return ValueType.valueOf(s2);
			}
			catch(IllegalArgumentException e)
			{
				System.out.printf("ERROR in file '%s' at line %d: unknown unit_type '%s'. Expected values are:%s\n", filename, i, s1, getPossibleValues(ValueType.values()));
				System.exit(-1);
				return null;
			}
			
		}
		
		return null;
	}
		
	
	private static void convertConfig(Map<ValueType, List<UnitType>> conversions, String filename, SystemType targetSystem, String filenameOutput)
	{
		List<PrefixType> prefixes = loadPrefixes();
		
		
		String[] input = loadFileContent(filename);
		
		ArrayList<String> output = new ArrayList<>();
		output.add(String.format("# Generated to %s units on %s by %s", targetSystem.name(), new Date().toString(), ConfigUnitConvertor.class.getSimpleName()));
		output.add("# ");
		
		
		
		ValueType currentValueType = null;
		
		boolean ignoreEnabledFlag = false;
		
		int i = 0;
		for (String line: input)
		{
			i++;
			
			String lineTrim = line.trim();
			if (lineTrim.isEmpty() || lineTrim.startsWith("#"))
			{
				output.add(line);
				
				if (getTemplate(line, filename, i) != null)
				{
					currentValueType = getTemplate(line, filename, i);
					if (currentValueType == ValueType.IGNORE_START)
					{
						ignoreEnabledFlag = true;
						currentValueType = null;
					}
					if (currentValueType == ValueType.IGNORE_END)
					{
						ignoreEnabledFlag = false;
						currentValueType = null;
					}
					
				}
			} 
			else
			{
				
				Matcher matcher1 = pattern1.matcher(line);
				if (!ignoreEnabledFlag && matcher1.matches())
				{
					String key = matcher1.group(1);
					
					// I assume I have the type here
					
					if (currentValueType == null)
					{
						System.out.printf("ERROR in file '%s' at line %d: unspecified unit_type for key '%s'.\n", filename, i, key);
						System.exit(-1);
						return;
					}
					else
					{
						if (currentValueType != ValueType.IGNORE)
						{
							Matcher matcher2 = pattern2.matcher(line);
							
							if (matcher2.matches())
							{
								
								
								String expKey = matcher2.group(1);
								String expValue = matcher2.group(2);
								String expUnit = matcher2.group(4);
								
								if (!key.equals(expKey))
								{
									System.out.printf("ERROR in file '%s' at line %d: internal assert 1 failed. Contact developer.\n", filename, i);
									System.exit(-1);
									return;
								}
								
								convertToSI(filename, i, currentValueType, conversions.get(currentValueType), prefixes, expKey, expValue, expUnit, output, targetSystem);
								
							}
							else
							{
								System.out.printf("ERROR in file '%s' at line %d: unable parse the line.\n", filename, i);
								System.exit(-1);
								return;
							}
						}	
						else
						{
							output.add(line);
						}
					}
				}
				else
				{
					output.add(line);
					currentValueType = null;
				}
				
				
				
				
			}
		}
		
		
		saveFileContent(filenameOutput, output);
		
	}
	

	private static void convertToSI(String filename, int i, ValueType currentTemplate, List<UnitType> possibleTypes, List<PrefixType> possiblePrefixes, String expKey, String expValue, String expPrefixAndUnit,	ArrayList<String> output, SystemType targetSystem)
	{
		if (currentTemplate == ValueType.PURE_INT && (expPrefixAndUnit == null || expPrefixAndUnit.isEmpty()))
		{
			// It is a pure number. Just copying
			try
			{
				output.add(String.format("%s\t\t = %d", expKey, Long.parseLong(expValue)));
			} 
			catch (NumberFormatException e)
			{
				System.out.printf("ERROR in file '%s' at line %d: unable parse %s value %s\n", filename, i, "integer", expValue);
				System.exit(-1);
				return;
			}
		}
		else if (currentTemplate == ValueType.PURE_FLOAT && (expPrefixAndUnit == null || expPrefixAndUnit.isEmpty()))
		{
			// It is a pure number. Just copying
			try
			{
				output.add(String.format("%s\t\t = %.16E", expKey, Double.parseDouble(expValue)));
			} 
			catch (NumberFormatException e)
			{
				System.out.printf("ERROR in file '%s' at line %d: unable parse %s value %s\n", filename, i, "float", expValue);
				System.exit(-1);
				return;
			}
		}
		else
		{
			
			if (expPrefixAndUnit == null || expPrefixAndUnit.isEmpty())
			{
				System.out.printf("ERROR in file '%s' at line %d: no unit was specified for key '%s'. Expected types are: %s\n", filename, i, expKey, getPossibleValues(possibleTypes));
				System.exit(-1);
				return;
			}
			
			UnitType unitMatch = null;
			
			for (UnitType type: possibleTypes)
			{
				if (expPrefixAndUnit.endsWith(type.symbol))
				{
					unitMatch = type;
					break;
				}
			}
			
			if (unitMatch == null)
			{
				System.out.printf("ERROR in file '%s' at line %d: unable to undestand the %s unit for the key '%s'. Unknown symbol '%s'.\nExpected symbols are:%s\n", 
						filename, 
						i,
						currentTemplate.name().toLowerCase(),
						expKey,
						expPrefixAndUnit,
						getPossibleValues(possibleTypes));
				System.exit(-1);
				return;
			}
			
			
			// We assume we found the right unit
			String expPrefix = expPrefixAndUnit.substring(0, expPrefixAndUnit.length() - unitMatch.symbol.length());
			
			
			PrefixType prefixMatch = null;
			
			if (expPrefix != null && !expPrefix.isEmpty())
			{
				
				for (PrefixType prefix: possiblePrefixes)
				{
					if (expPrefixAndUnit.startsWith(prefix.symbol))
					{
						prefixMatch = prefix;
						break;
					}
				}
				
				
				if (prefixMatch == null)
				{
									
					System.out.printf("ERROR in file '%s' at line %d: unable to undestand the prefix for the key '%s'.\nUnknown symbol '%s'.\nPossible prefixes are: %s\n", filename, i, currentTemplate.name().toLowerCase(), expPrefix, getPossibleValues(possiblePrefixes));
					System.exit(-1);
					return;
				}
				
			}
			
			// Here we are done. We do the conversion and write the file.
			// We add also a comment with the details of the conversion
			
			if (currentTemplate == ValueType.PURE_INT && prefixMatch != null)
			{
				long oldValue = Long.parseLong(expValue);
				long newValue = oldValue * 10 ^ prefixMatch.power;
				output.add(String.format("# converted %s from %d %s (%s) to %d", "integer", oldValue, prefixMatch.symbol, prefixMatch.name, newValue));
				output.add(String.format("%s\t\t = %d", newValue));
				
			}
			else if (currentTemplate == ValueType.PURE_FLOAT && prefixMatch != null)
			{
				Double oldValue = Double.parseDouble(expValue);
				Double newValue = oldValue * Math.pow(10, prefixMatch.power);
				output.add(String.format("# converted %s from %f %s (%s) to %.16E", "float", oldValue, prefixMatch.symbol, prefixMatch.name, newValue));
				output.add(String.format("%s\t\t = %.16E", expKey, newValue));
				
			}
			else
			{
				UnitType targetUnit = getTargetUnit(possibleTypes, targetSystem);
				
				if (targetUnit == null)
				{
					System.out.printf("ERROR in file '%s' at line %d: unable to find a %s unit representing an %s. Complete the 'conversion' file\n", filename, i, targetSystem.name(), currentTemplate.name().toLowerCase());
					System.exit(-1);
					return;
				}
				
				Double oldValue = Double.parseDouble(expValue);
				
				Double newValue = oldValue / targetUnit.siValue * unitMatch.siValue;
				
				if (prefixMatch != null)
					newValue = newValue * Math.pow(10, prefixMatch.power);
				
				
				output.add(String.format("# converted %s from %f %s%s (%s%s) to %.16E %s", 
						currentTemplate.name().toLowerCase(), 
						oldValue, 
						(prefixMatch != null ? prefixMatch.symbol : ""), 
						unitMatch.symbol,
						(prefixMatch != null ? prefixMatch.name : ""), 
						unitMatch.name,
						newValue,
						targetUnit.symbol));
				output.add(String.format("%s\t\t = %.16E", expKey, newValue));
			}
			
			
 			
			
		}
		
	}
	private static UnitType getTargetUnit(List<UnitType> possibleTypes, SystemType targetSystem)
	{
		UnitType targetUnit = null;
		
		for (UnitType possibleType: possibleTypes)
		{
			if (possibleType.systems.contains(targetSystem))
			{
				targetUnit = possibleType;
				break;
			}
			
		}
		return targetUnit;
	}
	
	
	public static void main(String[] args)
    {
		
		if (args.length == 3 && args[0].equals("--print-units"))
		{
			SystemType targetSystem = parseTarget(args[1]);
			String filenameConversions  = args[2];
	    	
	    	printUnits(targetSystem, loadConversions(filenameConversions));
	    	
		}
		else if (args.length == 5 && args[0].equals("--convert-cfg"))
		{
			SystemType targetSystem = parseTarget(args[1]);
			
	    	String filenameConversions  = args[2];
	    	String filenameConfig 		= args[3];
	    	String filenameOutput 		= args[4];
	    	
	    	Map<ValueType, List<UnitType>> conversion = loadConversions(filenameConversions);
	    	
	    	
	    	convertConfig(conversion, filenameConfig, targetSystem, filenameOutput);
		}
		else
		{
			System.out.println("Usage:");
			System.out.println("convert --convert-cfg <target_system> <conversion_file.csv> <input_file.cfg> <output_file.cfg>");
			System.out.println("convert --print-units <target_system> <conversion_file.csv>");
    
		}
    }
	private static SystemType parseTarget(String target)
	{
		SystemType targetSystem;
		try
		{
		  targetSystem = SystemType.valueOf(target.toUpperCase());
		}
		catch(IllegalArgumentException e)
		{
			System.out.printf("Unable to undestand the target system 's': allowed values are %s\n", target, getPossibleValues(SystemType.values()));
			System.exit(1);
			return null;
		}
		return targetSystem;
	}

	private static void printUnits(SystemType targetSystem, Map<ValueType, List<UnitType>> conversion)
	{
		for (ValueType value: ValueType.values())
		{
			if (value != null && value != ValueType.PURE_FLOAT && value != ValueType.PURE_INT && !value.name().startsWith("IGNORE"))
			{	
				List<UnitType> possibleTypes = conversion.get(value);
				
				
				UnitType targetUnit = getTargetUnit(possibleTypes, targetSystem);
				
				if (targetUnit == null)
				{
					System.out.printf("ERROR: unable to find a %s unit representing an %s. Complete the 'conversion' file.\n", targetSystem.name(), value);
					System.exit(-1);
					return;
				}
				
				System.out.println(String.format("%s:", value.name()));
				for (UnitType possibleType: possibleTypes)
				{
					String conversion1 = String.format("1 %-6s = % .16E %-6s", possibleType.symbol, possibleType.siValue / targetUnit.siValue, targetUnit.symbol);
					String conversion2 = String.format("1 %-6s = % .16E %-6s", targetUnit.symbol,	targetUnit.siValue / possibleType.siValue, possibleType.symbol);
							
							
					System.out.println(String.format("  %-40s: %-40s %-40s", possibleType.name, conversion1, conversion2));
				}
				System.out.println("");
			}
		}
		
	}
	
	private static String getPossibleValues(Object[] values)
	{
		return getPossibleValues(Arrays.asList(values));
	}
	private static String getPossibleValues(List<? extends Object> values)
	{
		StringBuffer buffer = new StringBuffer();
		
		for (Object value: values)
		{
			buffer.append("\n");
			buffer.append(value.toString());
		}
		
		return buffer.toString();
	}
	
}
