#include "loader.hpp"

#include "utilities.hpp"
#include "bitmask.hpp"
#include "printer.hpp"
#include "time.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

ByteReader::ByteReader(const uint8_t* data, size_t size) : position(data), end(data + size) {}

size_t ByteReader::BytesLeft() const
{
	return (CST(end - position));
}

size_t ByteReader::Offset(const uint8_t* base) const
{
	return (CST(position - base));
}

uint8_t ByteReader::Read8()
{
	if (position >= end) throw (std::runtime_error("Byte reader out of bounds (8)"));

	//return (*position++);

	uint8_t result = position[0];
	position++;
	
	return (result);
}

uint16_t ByteReader::Read16()
{
	if (end - position < 2) throw (std::runtime_error("Byte reader out of bounds (16)"));

	uint16_t result = ((C16(position[0]) << 8) | C16(position[1]));
	position += 2;

	return (result);
}

uint32_t ByteReader::Read32()
{
	if (end - position < 4) throw (std::runtime_error("Byte reader out of bounds (32)"));

	uint32_t result = ((C32(position[0]) << 24) | (C32(position[1]) << 16) | (C32(position[2]) << 8) | C32(position[3]));
	position += 4;

	return (result);
}

void ByteReader::Skip(size_t bytes)
{
	if (BytesLeft() < bytes) throw (std::runtime_error("Byte reader out of bounds (Skip)"));

	position += bytes;
}

bool ByteReader::AtMarker(ImageMarker marker)
{
	if (Read8() != 0xff) return (false);

	uint8_t byte = Read8();

	while (byte == 0xff) { byte = Read8(); }
	
	if (byte != C8(marker)) return (false);
	
	return (true);
}

ImageMarker ByteReader::NextMarker()
{
	while (Read8() != 0xff) { continue; }

	uint8_t byte = Read8();

	while (byte == 0xff) { byte = Read8(); }

	return (static_cast<ImageMarker>(byte));
}

point3D AttributeInfo::Translation() const
{
	point3D result;

	size_t start = 0;
	size_t end = translation.find(',');
	result.x() = std::stof(translation.substr(start, end - start));
	start = end + 1;
	end = translation.find(',');
	result.y() = std::stof(translation.substr(start, end - start));
	start = translation.rfind(',') + 1;
	result.z() = std::stof(translation.substr(start));

	return (result);
}

ModelLoader::ModelLoader(const std::string& name, const ModelType& type, size_t meshID)
{
	switch (type)
	{
		case ModelType::Obj: GetObjInfo(name, meshID); break;
		case ModelType::Gltf: GetGltfInfo(name, meshID); break;
		default: throw (std::runtime_error("Not a valid model type"));
	}
}

ModelLoader::~ModelLoader()
{

}

std::string ModelLoader::GetValue(const std::string& content, const std::string& target) const
{
	if (!content.contains(target)) return ("");

	size_t start = content.find(target) + target.size() + 3;
	size_t end = content.find('\n', start);

	return (content.substr(start, end - start));
}

AttributeInfo ModelLoader::GetAttribute(const std::string& accessContent, const std::string& viewContent) const
{
	AttributeInfo attributeInfo{};
	attributeInfo.accessContent = accessContent;
	attributeInfo.viewIndex = GetValue(attributeInfo.accessContent, "bufferView");
	attributeInfo.component = GetValue(attributeInfo.accessContent, "componentType");
	attributeInfo.count = GetValue(attributeInfo.accessContent, "count");
	attributeInfo.type = GetValue(attributeInfo.accessContent, "type");
	attributeInfo.viewContent = viewContent;
	attributeInfo.bufferIndex = GetValue(attributeInfo.viewContent, "buffer");
	attributeInfo.length = GetValue(attributeInfo.viewContent, "byteLength");
	attributeInfo.offset = GetValue(attributeInfo.viewContent, "byteOffset");

	return (attributeInfo);
}

std::string ModelLoader::GetPart(const std::string& content, const std::string& target, const std::pair<char, char>& pair)
{
	std::string result = "";

	std::pair<size_t, size_t> range = Utilities::FindPair(content, content.find(target), pair);
	if (range.first != range.second) result = content.substr(range.first + 1, range.second - range.first - 1);

	return (result);
}

std::vector<std::string> ModelLoader::GetList(const std::string& content, const std::string& target, const std::pair<char, char>& pair)
{
	std::vector<std::string> result;

	size_t end = 0;

	while (end != std::string::npos && end < content.size())
	{
		size_t start = end;
		if (target != "") start = content.find(target, end);
		std::pair<size_t, size_t> range = Utilities::FindPair(content, start, pair);
		if (range.first == range.second) break;
		if (range.first == std::string::npos || range.second == std::string::npos) break;
		if (range.first >= content.size() || range.second >= content.size()) break;
		result.push_back(content.substr(range.first + 1, range.second - range.first - 1));
		end = range.second + 1;
	}

	return (result);
}

void ModelLoader::GetObjInfo(const std::string& name, size_t meshID)
{
	info.name = name;
}

void ModelLoader::GetGltfInfo(const std::string& name, size_t meshID)
{
	info.name = name;
	info.type = ModelType::Gltf;

	std::string path = Utilities::GetPath() + "/resources/models/" + name + ".gltf";
	std::string file = Utilities::FileToString(path);

	std::string meshesInfo = GetPart(file, "meshes", {'[', ']'});

	std::vector<std::string> primitives = GetList(meshesInfo, "primitives", {'[', ']'});
	std::vector<std::string> meshes;

	for (std::string primitive : primitives)
	{
		std::vector<std::string> temp = GetList(primitive, "", {'{', '}'});
		meshes.insert(meshes.end(), temp.begin(), temp.end());
	}

	if (meshID >= meshes.size()) throw (std::runtime_error("Invalid mesh ID"));

	info.ID = meshID;
	info.count = meshes.size() - 1 - meshID;

	std::string positionsIndex = GetValue(meshes[meshID], "POSITION");
	std::string normalsIndex = GetValue(meshes[meshID], "NORMAL");
	std::string coordinatesIndex = GetValue(meshes[meshID], "TEXCOORD_0");
	std::string indicesIndex = GetValue(meshes[meshID], "indices");

	std::string accessInfo = GetPart(file, "accessors", {'[', ']'});
	std::vector<std::string> accessors = GetList(accessInfo, "", {'{', '}'});

	std::string viewInfo = GetPart(file, "bufferViews", {'[', ']'});
	std::vector<std::string> views = GetList(viewInfo, "", {'{', '}'});

	if (positionsIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Position);
		std::string accessContent = accessors[std::stoi(positionsIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Position] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Position].count));

		std::string nodeInfo = GetPart(file.substr(file.rfind("nodes")), "nodes", {'[', ']'});
		if (nodeInfo != "")
		{
			std::vector<std::string> nodes = GetList(nodeInfo, "", {'{', '}'});
			if (nodes.size() > meshID)
			{
				std::string translation = GetPart(nodes[meshID], "translation", {'[', ']'});
				if (translation != "") info.attributes[AttributeType::Position].translation = translation;
			}
		}
	}

	if (normalsIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Normal);
		std::string accessContent = accessors[std::stoi(normalsIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Normal] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Normal].count));
	}

	if (coordinatesIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Coordinate);
		std::string accessContent = accessors[std::stoi(coordinatesIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Coordinate] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Coordinate].count));
	}

	if (indicesIndex != "")
	{
		info.indexConfig = VK_INDEX_TYPE_UINT16;
		std::string accessContent = accessors[std::stoi(indicesIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Index] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Index].count));
	}
}

const ModelInfo& ModelLoader::GetInfo() const
{
	return (info);
}

const AttributeInfo& ModelInfo::GetAttribute(const AttributeType& type) const
{
	return (attributes.at(type));
}

void ModelLoader::GetBytes(char* address, const AttributeType& type)
{
	if (!info.attributes.contains(type)) throw (std::runtime_error("Model does not contain attribute type"));

	std::string path = Utilities::GetPath() + "/resources/models/" + info.name + ".bin";
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open()) throw (std::runtime_error("Failed to open file: " + info.name));

	file.seekg(info.attributes[type].Offset());
	file.read(address, info.attributes[type].Length());

	file.close();
}

ImageLoader::ImageLoader(const std::string& name, const ImageType& type)
{
	switch (type)
	{
		case ImageType::Jpg: GetJpgInfo(name); break;
		case ImageType::Png: return; break;
		default: throw (std::runtime_error("Not a valid image type"));
	}
}

ImageLoader::~ImageLoader()
{

}

void ImageLoader::GetJpgInfo(const std::string& name)
{
	info.name = name;
	info.type = ImageType::Jpg;

	std::string path = Utilities::GetPath() + "/resources/textures/" + name + ".jpg";
	std::string file = Utilities::FileToString(path);
	const uint8_t* rawData = reinterpret_cast<const uint8_t*>(file.c_str());

	ByteReader br(rawData, file.size());

	if (!br.AtMarker(ImageMarker::SOI)) throw (std::runtime_error("Invalid JPG file."));

	while (br.BytesLeft() > 1)
	{
		ImageMarker next = br.NextMarker();

		if (next == ImageMarker::SOF0)
		{
			//info.startOfFrameInfo.start = br.BytesLeft();
			info.startOfFrameInfo.start = br.Offset(rawData);
			info.startOfFrameInfo.length = CST(br.Read16());
			info.startOfFrameInfo.precision = CST(br.Read8());
			info.startOfFrameInfo.height = CST(br.Read16());
			info.startOfFrameInfo.width = CST(br.Read16());
			info.startOfFrameInfo.componentCount = CST(br.Read8());
			info.startOfFrameInfo.components.resize(info.startOfFrameInfo.componentCount);
			for (Point<size_t, 4>& component : info.startOfFrameInfo.components)
			{
				component.x() = CST(br.Read8());

				uint8_t HV = br.Read8();
				component.y() = HV >> 4;
				component.z() = HV & 0x0F;

				component.w() = CST(br.Read8());
			}

			if (info.startOfFrameInfo.start + info.startOfFrameInfo.length > br.Offset(rawData))
			{
				br.Skip((info.startOfFrameInfo.start + info.startOfFrameInfo.length) - br.Offset(rawData));
			}

			
		}
		else if (next == ImageMarker::EOI)
		{
			//std::cout << "EOI: " << br.BytesLeft() << std::endl;
			//std::cout << std::endl;
		}
		else if ((next >= ImageMarker::APP0 && next <= ImageMarker::APP15) || next == ImageMarker::COM)
		{
			size_t length = CST(br.Read16()) - 2;
			//std::cout << "Skipping: " << length << std::endl << std::endl;
			br.Skip(length);
		}
		else if (next == ImageMarker::DQT)
		{
			DQTInfo quantizationTable{};

			quantizationTable.start = br.Offset(rawData);
			quantizationTable.length = CST(br.Read16());

			uint8_t PT = br.Read8();
			quantizationTable.precision = CST(PT >> 4);
			quantizationTable.ID = CST(PT & 0x0F);

			for (size_t i = 0; i < 64; i++)
			{
				quantizationTable.values[zigzagTable[i]] = C16(br.Read8());
			}

			info.quantizationTables.push_back(quantizationTable);

			if (quantizationTable.start + quantizationTable.length > br.Offset(rawData))
			{
				br.Skip((quantizationTable.start + quantizationTable.length) - br.Offset(rawData));
			}
		}
		else if (next == ImageMarker::DHT)
		{
			DHTInfo huffmanInfo{};

			huffmanInfo.start = br.Offset(rawData);
			huffmanInfo.length = CST(br.Read16());

			uint8_t CH = br.Read8();
			huffmanInfo.type = CST(CH >> 4);
			huffmanInfo.ID = CST(CH & 0x0F);

			uint8_t counts[16]{};
			size_t total = 0;
			for (size_t i = 0; i < 16; i++)
			{
				counts[i] = br.Read8();
				total += CST(counts[i]);
			}

			std::vector<uint8_t> symbols(total);
			for (size_t i = 0; i < total; i++)
			{
				symbols[i] = br.Read8();
			}

			uint16_t code = 0;
			uint16_t firstCodes[16]{};
			for (size_t i = 0; i < 16; i++)
			{
				firstCodes[i] = code;
				code = (code + counts[i]) << 1;
			}

			size_t symbolIndex = 0;
			size_t symbolCode = 0;
			for (size_t i = 0; i < 16; i++)
			{
				symbolCode = firstCodes[i];
				for (size_t j = 0; j < counts[i]; j++)
				{
					HuffmanCode huffmanCode{};
					huffmanCode.symbol = symbols[symbolIndex];
					huffmanCode.length = i + 1;
					huffmanCode.code = symbolCode;
					symbolIndex++;
					symbolCode++;
					huffmanInfo.huffmanCodes.push_back(huffmanCode);
				}
			}

			info.huffmanInfos.push_back(huffmanInfo);

			if (huffmanInfo.start + huffmanInfo.length > br.Offset(rawData))
			{
				br.Skip((huffmanInfo.start + huffmanInfo.length) - br.Offset(rawData));
			}
		}
		else if (next == ImageMarker::DRI)
		{
			size_t length = CST(br.Read16()) - 2;
			br.Skip(length);
		}
		else if (next == ImageMarker::SOS)
		{
			info.startOfScanInfo.start = br.Offset(rawData);
			info.startOfScanInfo.length = CST(br.Read16());
			info.startOfScanInfo.componentCount = CST(br.Read8());
			for (size_t i = 0; i < info.startOfScanInfo.componentCount; i++)
			{
				size_t componentID = CST(br.Read8());
				uint8_t AD = CST(br.Read8());
				std::pair<size_t, size_t> tables{ CST(AD >> 4), CST(AD & 0x0F)};
				info.startOfScanInfo.componentTables[componentID] = tables;
			}

			if (info.startOfScanInfo.start + info.startOfScanInfo.length > br.Offset(rawData))
			{
				br.Skip((info.startOfScanInfo.start + info.startOfScanInfo.length) - br.Offset(rawData));
			}
		}
	}
}

void ImageLoader::BuildHuffmanTree(std::string current, HuffmanTree& start, std::vector<HuffmanCode>& codes)
{
	if (current.size() >= 16) return;

	std::string leftCode = current + "0";
	std::string rightCode = current + "1";

	for (const HuffmanCode& code : codes)
	{
		std::string codeString = Utilities::ToBits(code.code).substr(16 - CST(code.length));

		if (codeString == leftCode)
		{
			start.Expand({true, code}, Left);
		}
		else if (codeString == rightCode)
		{
			start.Expand({true, code}, Right);
		}
	}

	if (!start.HasSide(Left))
	{
		start.Expand({false, {}}, Left);
		BuildHuffmanTree(leftCode, *start.GetSide(Left), codes);
	}
	if (!start.HasSide(Right))
	{
		start.Expand({false, {}}, Right);
		BuildHuffmanTree(rightCode, *start.GetSide(Right), codes);
	}
}

std::array<int16_t, 1 << FAST_BITS> ImageLoader::BuildFastHuffmanTable(std::vector<HuffmanCode>& codes)
{
	std::array<int16_t, 1 << FAST_BITS> table{};

	for (size_t i = 0; i < (1 << FAST_BITS); i++) table[i] = -1;

	for (size_t i = 0; i < codes.size(); i++)
	{
		int length = codes[i].length;
		if (length <= FAST_BITS)
		{
			int shift = FAST_BITS - length;
			int base = codes[i].code << shift;
			int repetitions = 1 << shift;

			for (size_t r = 0; r < repetitions; r++)
			{
				table[base + r] = i;
			}
		}
	}

	return (table);
}

/*DataBlock ImageLoader::IDCTBlock(const DataBlock& input)
{
	DataBlock result{};

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			float sum = 0;

			for (int v = 0; v < 8; v++)
			{
				for (int u = 0; u < 8; u++)
				{
					float AU = (u == 0) ? sqrt12 : 1;
					float AV = (v == 0) ? sqrt12 : 1;
					sum += AU * AV * input[v * 8 + u] * 
						cos(((2 * x + 1) * u * M_PI) / 16.0) * 
						cos(((2 * y + 1) * v * M_PI) / 16.0);
				}
			}

			int16_t intSum = static_cast<int16_t>(sum * 0.25);
			intSum += 128;
			if (intSum < 0) intSum = 0;
			if (intSum > 255) intSum = 255;

			result[y * 8 + x] = intSum;
		}
	}

	return (result);
}*/

DataBlock ImageLoader::FIDCTBlock(const DataBlock& input)
{
	static std::array<float, 64> transform{};
	static bool computed = false;

	if (!computed)
	{
		computed = true;
		for (int v = 0; v < 8; v++)
		{
			for (int u = 0; u < 8; u++)
			{
				float AU = (u == 0) ? sqrt12 : 1;
				transform[v * 8 + u] = 0.5f * AU * cos(((2 * v + 1) * u * M_PI) / 16.0);
			}
		}
	}

	DataBlock result{};
	std::array<float, 64> temp{};

	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			float sum = 0;

			for (int k = 0; k < 8; k++)
			{
				sum += transform[y * 8 + k] * input[k * 8 + x];
			}

			temp[y * 8 + x] = sum;
		}
	}

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			float sum = 0;

			for (int k = 0; k < 8; k++)
			{
				sum += transform[x * 8 + k] * temp[y * 8 + k];
			}

			int intSum = std::lround(sum + 128);
			if (intSum < 0) intSum = 0;
			if (intSum > 255) intSum = 255;

			result[y * 8 + x] = static_cast<int16_t>(intSum);
		}
	}

	return (result);
}

const ImageInfo& ImageLoader::GetInfo() const
{
	return (info);
}

uint8_t ImageLoader::NextEntropySymbol(EntropyReader& er, size_t tableIndex)
{
	//const std::pair<bool, uint8_t> fastResult = er.NextSymbolFast(data.fastHuffmanTables[tableIndex], info.huffmanInfos[tableIndex].huffmanCodes);
	//
	//if (fastResult.first)
	//{
	//	//fastFound++;
	//	return (fastResult.second);
	//}
	//else
	//{
	//	//slowFound++;
	//	return (er.NextSymbolFast(data.huffmanTables[tableIndex]));
	//}

	return (er.NextSymbolFast(data.huffmanTables[tableIndex]));
}

static double subTime = 0;
static double addTime = 0;
static double restTime = 0;

void ImageLoader::LoadEntropyData()
{
	double start = Time::GetCurrentTime();

	size_t totalBlockCount = 0;
	size_t maxH = 0;
	size_t maxV = 0;
	for (const Point<size_t, 4>& component : info.startOfFrameInfo.components)
	{
		totalBlockCount += component.y() * component.z();
		maxH = std::max(maxH, component.y());
		maxV = std::max(maxV, component.z());
	}
	data.MCUCount.x() = std::ceil(info.startOfFrameInfo.width / (maxH * 8));
	data.MCUCount.y() = std::ceil(info.startOfFrameInfo.height / (maxV * 8));
	data.MCUCount.z() = data.MCUCount.x() * data.MCUCount.y();

	data.blocks.resize(totalBlockCount * data.MCUCount.z());
	for (size_t i = 0; i < data.blocks.size(); i++)
	{
		data.blocks[i] = DataBlock{};
	}

	double treeStart = Time::GetCurrentTime();

	data.huffmanTables.resize(info.huffmanInfos.size(), HuffmanTree({false, {}}));
	data.fastHuffmanTables.resize(info.huffmanInfos.size());
	size_t HI = 0;
	for (HuffmanTree& table : data.huffmanTables)
	{
		BuildHuffmanTree("", table, info.huffmanInfos[HI].huffmanCodes);
		data.fastHuffmanTables[HI] = BuildFastHuffmanTable(info.huffmanInfos[HI].huffmanCodes);
		//data.huffmanTableInfos.push_back({table});
		HI++;
	}

	std::cout << "Trees build in: " << (Time::GetCurrentTime() - treeStart) * 1000 << std::endl;

	double fileStart = Time::GetCurrentTime();

	std::string path = Utilities::GetPath() + "/resources/textures/" + info.name + ".jpg";
	std::string file = Utilities::FileToString(path);
	const uint8_t* rawData = reinterpret_cast<const uint8_t*>(file.c_str());

	std::cout << "File loaded in: " << (Time::GetCurrentTime() - fileStart) * 1000 << std::endl;

	ByteReader br(rawData, file.size());
	br.Skip(info.startOfScanInfo.start + info.startOfScanInfo.length);

	size_t blockIndex = 0;
	uint8_t symbol;
	int value;
	//int DCs[info.startOfFrameInfo.components.size()]{};
	std::vector<int> DCs(info.startOfFrameInfo.components.size());

	EntropyReader er(br);

	double entropyStart = Time::GetCurrentTime();

	double treeTime = 0;
	double transformTime = 0;
	double readTime = 0;

	for (size_t MCU = 0; MCU < data.MCUCount.z(); MCU++)
	{
		size_t componentIndex = 0;
		for (auto table : info.startOfScanInfo.componentTables)
		{
			Point<size_t, 4> currentComponent;
			for (Point<size_t, 4> componentInfo : info.startOfFrameInfo.components)
			{
				if (componentInfo.x() == table.first)
				{
					currentComponent = componentInfo;
					break;
				}
			}

			int DCIndex = -1;
			int ACIndex = -1;

			for (size_t i = 0; i < info.huffmanInfos.size(); i++)
			{
				if (info.huffmanInfos[i].type == 0 && info.huffmanInfos[i].ID == table.second.first) DCIndex = i;
				if (info.huffmanInfos[i].type == 1 && info.huffmanInfos[i].ID == table.second.second) ACIndex = i;
			}

			if (DCIndex == -1 || ACIndex == -1) throw (std::runtime_error("No valid tables found"));

			size_t blockCount = currentComponent.y() * currentComponent.z();
			for (size_t BI = 0; BI < blockCount; BI++)
			{
				double treeTimeStart = Time::GetCurrentTime();

				//symbol = er.NextSymbolFast(data.huffmanTables[DCIndex]);
				symbol = NextEntropySymbol(er, DCIndex);

				treeTime += (Time::GetCurrentTime() - treeTimeStart);

				double readTimeStart = Time::GetCurrentTime();

				//if (symbol > 0)	{ value = er.ReadBits(symbol); }
				if (symbol > 0)	{ value = er.ReadBitsBuffer(symbol); }
				else { value = 0; }

				readTime += (Time::GetCurrentTime() - readTimeStart);

				DCs[componentIndex] += value;

				data.blocks[blockIndex][0] = static_cast<int16_t>(DCs[componentIndex]  * info.quantizationTables[currentComponent.w()].values[0]);

				for (size_t i = 1; i < 64; i++)
				{
					treeTimeStart = Time::GetCurrentTime();

					//symbol = er.NextSymbolFast(data.huffmanTables[ACIndex]);
					symbol = NextEntropySymbol(er, ACIndex);

					treeTime += (Time::GetCurrentTime() - treeTimeStart);

					if (symbol == 0x00) break;
					if (symbol == 0xF0)
					{
						for (size_t j = 0; j < 16; j++)
						{
							if (i >= 64) throw (std::runtime_error("16 zero run length: out of block bounds"));

							data.blocks[blockIndex][zigzagTable[i]] = 0;
							if (j + 1 < 16) i++;
						}
						continue;
					}

					int run = symbol >> 4;
					int size = symbol & 0x0F;

					if (size == 0) throw (std::runtime_error("size equals zero"));

					for (int j = 0; j < run; j++)
					{
						if (i >= 64) throw (std::runtime_error("size run length: out of block bounds"));

						data.blocks[blockIndex][zigzagTable[i]] = 0;
						i++;
					}

					readTimeStart = Time::GetCurrentTime();

					//value = er.ReadBits(size) * info.quantizationTables[currentComponent.w()].values[zigzagTable[i]];
					value = er.ReadBitsBuffer(size) * info.quantizationTables[currentComponent.w()].values[zigzagTable[i]];

					readTime += (Time::GetCurrentTime() - readTimeStart);

					data.blocks[blockIndex][zigzagTable[i]] = static_cast<int16_t>(value);
				}

				//for (size_t i = 0; i < 64; i++)
				//{
				//	data.blocks[blockIndex][i] *= info.quantizationTables[currentComponent.w()].values[i];
				//}

				double transformTimeStart = Time::GetCurrentTime();

				//data.blocks[blockIndex] = IDCTBlock(data.blocks[blockIndex]);
				data.blocks[blockIndex] = FIDCTBlock(data.blocks[blockIndex]);

				transformTime += (Time::GetCurrentTime() - transformTimeStart);

				blockIndex++;
			}

			componentIndex++;
		}
	}

	std::cout << "Tree search time: " << treeTime * 1000 << std::endl;

	std::cout << "Read time: " << readTime * 1000 << std::endl;

	std::cout << "Transform time: " << transformTime * 1000 << std::endl;

	std::cout << "Entropy loaded in: " << (Time::GetCurrentTime() - entropyStart) * 1000 << std::endl;

	std::cout << "Completed in: " << (Time::GetCurrentTime() - start) * 1000 << std::endl;

	//std::cout << "Fast found: " << fastFound << " Slow found: " << slowFound << std::endl;
	std::cout << "Add time: " << addTime * 1000 << " Sub time: " << subTime * 1000 << " Rest time: " << restTime * 1000 << std::endl;
}

void ImageLoader::LoadBlock(std::array<unsigned char, (16 * 16) * 4>& buffer, size_t offset)
{
	size_t pixelIndex = 0;
	size_t blockIndex = 0;

	for (size_t y = 0; y < 16; y++)
	{
		for (size_t x = 0; x < 16; x++)
		{
			int yi = y;
			int xi = x;
			int yci = y / 2;
			int xci = x / 2;
			if (y < 8 && x < 8)
			{
				blockIndex = 0;
			}
			else if (y < 8 && x >= 8)
			{
				blockIndex = 1;
				xi -= 8;
			}
			else if (y >= 8 && x < 8)
			{
				blockIndex = 2;
				yi -= 8;
			}
			else
			{
				blockIndex = 3;
				xi -= 8;
				yi -= 8;
			}

			double Y = static_cast<double>(data.blocks[blockIndex + (6 * offset)][yi * 8 + xi]);
			double Cb = static_cast<double>(data.blocks[4 + (6 * offset)][yci * 8 + xci]);
			double Cr = static_cast<double>(data.blocks[5 + (6 * offset)][yci * 8 + xci]);

			double R = (Y + 1.402 * (Cr - 128));
			double G = (Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128));
			double B = (Y + 1.772 * (Cb - 128));

			if (R < 0) R = 0;
			if (R > 255) R = 255;
			if (G < 0) G = 0;
			if (G > 255) G = 255;
			if (B < 0) B = 0;
			if (B > 255) B = 255;

			unsigned char r = static_cast<unsigned char>(R);
			unsigned char g = static_cast<unsigned char>(G);
			unsigned char b = static_cast<unsigned char>(B);

			buffer[pixelIndex++] = r;
			buffer[pixelIndex++] = g;
			buffer[pixelIndex++] = b;
			buffer[pixelIndex++] = 255;
		}
	}
}

void ImageLoader::LoadPixels(std::vector<unsigned char>& buffer)
{
	buffer.resize((info.startOfFrameInfo.width * info.startOfFrameInfo.height) * 4);

	for (size_t y = 0; y < data.MCUCount.y(); y++)
	{
		for (size_t x = 0; x < data.MCUCount.x(); x++)
		{
			std::array<unsigned char, (16 * 16) * 4> blockPixels{};
			LoadBlock(blockPixels, y * data.MCUCount.x() + x);

			for (size_t by = 0; by < 16; by++)
			{
				for (size_t bx = 0; bx < 16; bx++)
				{

					size_t bufferIndex = (y * 16 + by) * info.startOfFrameInfo.width * 4 + (x * 16 + bx) * 4;
					size_t blockIndex = by * 16 * 4 + bx * 4;

					buffer[bufferIndex++] = blockPixels[blockIndex++];
					buffer[bufferIndex++] = blockPixels[blockIndex++];
					buffer[bufferIndex++] = blockPixels[blockIndex++];
					buffer[bufferIndex++] = blockPixels[blockIndex++];
				}
			}
		}
	}
}

EntropyReader::EntropyReader(ByteReader& br) : br(br)
{
	
}

int EntropyReader::Extend(int v, int n)
{
	int vt = 1 << (n - 1);
	return (v < vt) ? (v - ((1 << n) - 1)) : v;
}

void EntropyReader::AddBitsBuffer()
{			
	uint8_t byte = br.Read8();

	if (byte == 0x00 && previous == 0xFF)
	{
		previous = byte;
		AddBitsBuffer();
		return;
	}

	previous = byte;

	bitBuffer = ((bitBuffer << 8) | byte);

	index -= 8;
}

void EntropyReader::ReadBitBuffer()
{
	while (index >= 8) AddBitsBuffer();

	bitValue = (bitBuffer & (1 << (15 - index))) != 0;
	index++;
}

HuffmanResult EntropyReader::FindCode(const std::string& code, HuffmanTree& root) const
{
	HuffmanResult result = {false, {}};
	HuffmanTree* node = &root;

	for (size_t i = 0; i < code.size(); i++)
	{
		BinaryTreeSide side = (code[i] == '0' ? Left : Right);

		if (!node->HasSide(side)) return (result);

		node = node->GetSide(side);
		if (node->GetValue().first && (i + 1 == code.size())) return (node->GetValue());
	}

	return (result);
}

uint8_t EntropyReader::NextSymbolFast(const HuffmanTree& tree)
{
	const HuffmanTree* node = &tree;

	while (true)
	{
		ReadBitBuffer();

		node = node->GetSide(bitValue ? Right : Left);
		HuffmanResult result = node->GetValue();
		if (result.first)
		{
			return (result.second.symbol);
		}
	}
}

std::pair<bool, uint8_t> EntropyReader::NextSymbolFast(const std::array<int16_t, 1 << FAST_BITS>& table, const std::vector<HuffmanCode>& codes)
{
	while (index >= 8) AddBitsBuffer();

	uint16_t val = bitBuffer;

	val = val << index;
	int key = (val >> (16 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
	int fastResult = table[key];

	if (fastResult >= 0)
	{
		int length = codes[fastResult].length;
		index += length;
		return (std::pair<bool, uint8_t>{true, codes[fastResult].symbol});
	}

	return (std::pair<bool, uint8_t>{false, 0});
}

int EntropyReader::ReadBitsBuffer(size_t amount)
{
	int result = 0;

	for (size_t i = 0; i < amount; i++)
	{
		ReadBitBuffer();
		if (bitValue) result += pow(2, (amount - (i + 1)));
	}

	return (Extend(result, amount));
}

std::ostream& operator<<(std::ostream& out, const ImageInfo& info)
{
	out << VAR_VAL(info.startOfFrameInfo.start) << std::endl;
	out << VAR_VAL(info.startOfFrameInfo.length) << std::endl;
	out << VAR_VAL(info.startOfFrameInfo.precision) << std::endl;
	out << VAR_VAL(info.startOfFrameInfo.height) << std::endl;
	out << VAR_VAL(info.startOfFrameInfo.width) << std::endl;
	out << VAR_VAL(info.startOfFrameInfo.componentCount) << std::endl;
	for (const point4D& component : info.startOfFrameInfo.components)
	{
		out << component << std::endl;
	}
	out << std::endl;

	for (const DQTInfo& quantizationTable : info.quantizationTables)
	{
		out << VAR_VAL(quantizationTable.start) << std::endl;
		out << VAR_VAL(quantizationTable.length) << std::endl;
		out << VAR_VAL(quantizationTable.precision) << std::endl;
		out << VAR_VAL(quantizationTable.ID) << std::endl;
		for (size_t r = 0; r < 8; r++)
		{
			for (size_t c = 0; c < 8; c++)
			{
				out << CST(quantizationTable.values[r * 8 + c]);
			}
			out << std::endl;
		}
		out << std::endl;
	}

	for (const DHTInfo& huffmanInfo : info.huffmanInfos)
	{
		out << VAR_VAL(huffmanInfo.start) << std::endl;
		out << VAR_VAL(huffmanInfo.length) << std::endl;
		out << VAR_VAL(huffmanInfo.type) << std::endl;
		out << VAR_VAL(huffmanInfo.ID) << std::endl;
		out << std::endl;
	}

	out << VAR_VAL(info.startOfScanInfo.start) << std::endl;
	out << VAR_VAL(info.startOfScanInfo.length) << std::endl;
	out << VAR_VAL(info.startOfScanInfo.componentCount) << std::endl;
	for (auto tables : info.startOfScanInfo.componentTables)
	{
		out << VAR_VAL(tables.first) << std::endl;
		out << VAR_VAL(tables.second.first) << " | " << VAR_VAL(tables.second.second) << std::endl;
	}
	out << std::endl;

	return (out);
}