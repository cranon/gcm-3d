#include "TetrMeshSecondOrder.h"

gcm::TetrMeshSecondOrder::TetrMeshSecondOrder() {
	tetrs2 = NULL;
	border2 = NULL;
	secondOrderNodesAreGenerated = false;
	triSizeInBytes = sizeof(TriangleSecondOrder);
	tetrSizeInBytes = sizeof(TetrSecondOrder);
	numericalMethodOrder = 2;
	INIT_LOGGER("gcm.TetrMeshSecondOrder");
}

gcm::TetrMeshSecondOrder::~TetrMeshSecondOrder() {
}

void gcm::TetrMeshSecondOrder::createTetrs(int number) {
	delete[] tetrs2;
	tetrs2 = new TetrSecondOrder[(int)(number*STORAGE_OVERCOMMIT_RATIO)];
	tetrs1 = tetrs2;
	tetrsNumber = 0;
	tetrsStorageSize = number*STORAGE_OVERCOMMIT_RATIO;
}

void gcm::TetrMeshSecondOrder::createTriangles(int number) {
	delete[] border2;
	border2 = new TriangleSecondOrder[number];
	border1 = border2;
	faceNumber = number;
}

TetrFirstOrder* gcm::TetrMeshSecondOrder::getTetr(int index) {
	assert( index >= 0 );
	map<int, int>::const_iterator itr;
	itr = tetrsMap.find(index);
	return ( itr != tetrsMap.end() ? 
		(TetrFirstOrder*)((char*)tetrs2 + itr->second * tetrSizeInBytes) : NULL );
	//if( tetrsMap.find(index) == tetrsMap.end() )
	//	return NULL;
	//return (TetrFirstOrder*)((char*)tetrs2 + tetrsMap[index] * tetrSizeInBytes);
}

TetrSecondOrder* gcm::TetrMeshSecondOrder::getTetr2(int index) {
	assert( index >= 0 );
	map<int, int>::const_iterator itr;
	itr = tetrsMap.find(index);
	return ( itr != tetrsMap.end() ? 
		(TetrSecondOrder*)((char*)tetrs2 + itr->second * tetrSizeInBytes) : NULL );
	//if( tetrsMap.find(index) == tetrsMap.end() )
	//	return NULL;
	//return (TetrSecondOrder*)((char*)tetrs2 + tetrsMap[index] * tetrSizeInBytes);
}

TetrFirstOrder* gcm::TetrMeshSecondOrder::getTetrByLocalIndex(int index) {
	assert( index >= 0 );
	return (TetrFirstOrder*)((char*)tetrs2 + index * tetrSizeInBytes);
}

TetrSecondOrder* gcm::TetrMeshSecondOrder::getTetr2ByLocalIndex(int index) {
	assert( index >= 0 );
	return (TetrSecondOrder*)((char*)tetrs2 + index * tetrSizeInBytes);
}

void gcm::TetrMeshSecondOrder::addTetr(TetrFirstOrder* tetr) {
	assert( tetrsNumber < tetrsStorageSize );
	tetrs2[tetrsNumber].number = tetr->number;
	memcpy(tetrs2[tetrsNumber].verts, tetr->verts, 4*sizeof(int));
	tetrsMap[tetr->number] = tetrsNumber;
	tetrsNumber++;
}

void gcm::TetrMeshSecondOrder::addTetr2(TetrSecondOrder* tetr) {
	assert( tetrsNumber < tetrsStorageSize );
	tetrs2[tetrsNumber] = *tetr;
	tetrsMap[tetr->number] = tetrsNumber;
	tetrsNumber++;
}

TriangleFirstOrder* gcm::TetrMeshSecondOrder::getTriangle(int index) {
	assert( index >= 0 );
	return (TriangleFirstOrder*)((char*)border2 + index * triSizeInBytes);
}

TriangleSecondOrder* gcm::TetrMeshSecondOrder::getTriangle2(int index) {
	assert( index >= 0 );
	return (TriangleSecondOrder*)((char*)border2 + index * triSizeInBytes);
}

void gcm::TetrMeshSecondOrder::copyMesh(TetrMeshFirstOrder* src)
{
	LOG_INFO("Creating mesh using copy");
	firstOrderNodesNumber = src->getNodesNumber();
	secondOrderNodesNumber = countSecondOrderNodes(src);
	
	createNodes(firstOrderNodesNumber + secondOrderNodesNumber);
	for( int i = 0; i < firstOrderNodesNumber; i++ )
		addNode( src->getNode(i) );
	
	createTetrs(src->getTetrsNumber());
	//for( int i = 0; i < tetrsNumber; i++ )
	for( MapIter itr = tetrsMap.begin(); itr != tetrsMap.end(); ++itr ) {
		int i = itr->first;
		addTetr( src->getTetr(i) );
	}
	for( int i = 0; i < src->getTetrsNumber(); i++ )
	{
		addTetr( src->getTetrByLocalIndex(i) );
		
	}
	
	generateSecondOrderNodes();
}

void gcm::TetrMeshSecondOrder::copyMesh2(TetrMeshSecondOrder* src)
{
	LOG_INFO("Creating second order mesh using copy");
	
	LOG_DEBUG("Nodes: " << src->getNodesNumber());
	createNodes( src->getNodesNumber() );
	for( int i = 0; i < src->getNodesNumber(); i++ )
		addNode( src->getNodeByLocalIndex(i) );
	
	LOG_DEBUG("Tetrs: " << src->getTetrsNumber());
	createTetrs(src->getTetrsNumber());
	//for( int i = 0; i < tetrsNumber; i++ )
	for( MapIter itr = tetrsMap.begin(); itr != tetrsMap.end(); ++itr ) {
		int i = itr->first;
		addTetr2( src->getTetr2(i) );
	}
	for( int i = 0; i < src->getTetrsNumber(); i++ )
	{
		addTetr2( src->getTetr2ByLocalIndex(i) );
	}
}

void gcm::TetrMeshSecondOrder::preProcess()
{
	LOG_DEBUG("Preprocessing second order mesh started");
	
	initNewNodes();
	create_outline();
	calc_min_h();
	calc_max_h();
	calc_avg_h();
	mesh_min_h *= 0.5;
	mesh_max_h *= 0.5;
	mesh_avg_h *= 0.5;
	
	verifyTetrahedraVertices();
	build_volume_reverse_lookups();

	/*LOG_DEBUG("Nodes map:");
	for( int i = 0; i < nodesNumber; i++ )
	{
		LOG_DEBUG("Node " << i << ": index " << nodesMap[i]);
		LOG_DEBUG("\t\tElements addr: " << getNode(i)->elements );
	}*/
	
	check_unused_nodes();	
	build_first_order_border();
	generateSecondOrderBorder();
	build_surface_reverse_lookups();

	check_numbering();
	check_outer_normals();
	LOG_DEBUG("Preprocessing mesh done.");
	
	logMeshStats();
}

void gcm::TetrMeshSecondOrder::verifyTetrahedraVertices ()
{
	LOG_DEBUG ("Verifying second order tetrahedra vertices");
	for (int tetrInd = 0; tetrInd < tetrsNumber; tetrInd++)
	{
		TetrSecondOrder* tetr = getTetr2ByLocalIndex(tetrInd);
		for (int vertInd = 0; vertInd < 4; vertInd++)
		{
			int nodeNum = tetr->verts[vertInd];
			assert( nodeNum >= 0 );
			assert( getNode( nodeNum )->isFirstOrder() );
		}

		for (int addVertInd = 0; addVertInd < 6; addVertInd++)
		{
			int addNodeNum = tetr->addVerts[addVertInd];
			assert( addNodeNum >= 0 );
			assert( getNode( addNodeNum )->isSecondOrder() );
		}
	}
}

void gcm::TetrMeshSecondOrder::build_volume_reverse_lookups()
{
	LOG_DEBUG("Building volume reverse lookups for second order mesh");

	ElasticNode* node;
	TetrFirstOrder* tetr;
	TetrSecondOrder* tetr2;
	// Init vectors for "reverse lookups" of tetrahedrons current node is a member of.
	//for(int i = 0; i < nodesNumber; i++) { 
	for( MapIter itr = nodesMap.begin(); itr != nodesMap.end(); ++itr ) {
		int i = itr->first;
		node = getNode(i);
		node->elements->clear();
		//delete node->elements;
		//node->elements = new vector<int>;
	}

	// Go through all the tetrahedrons
	//for(int i = 0; i < tetrsNumber; i++) {
	for( MapIter itr = tetrsMap.begin(); itr != tetrsMap.end(); ++itr ) {
		int i = itr->first;
		// For all verticles
		for(int j = 0; j < 4; j++)
		{
			tetr = getTetr(i);
			int nodeInd = tetr->verts[j];
			assert( getNode(nodeInd)->isFirstOrder() );
			// Push to data of nodes the number of this tetrahedron
			getNode( nodeInd )->elements->push_back( tetr->number );
		}
		for(int j = 0; j < 6; j++)
		{
			tetr2 = getTetr2(i);
			int nodeInd = tetr2->addVerts[j];
			assert( getNode(nodeInd)->isSecondOrder() );
			// Push to data of nodes the number of this tetrahedron
			getNode( nodeInd )->elements->push_back( tetr2->number );
		}
	}
	
	for( MapIter itr = nodesMap.begin(); itr != nodesMap.end(); ++itr ) {
		ElasticNode* node = getNode(itr->first);
		int num = node->elements->size();
		if( num <= 0 )
			LOG_WARN("Node is not a part of volumes. Node: " << *node);
	}
	/*for(int i = 0; i < nodesNumber; i++)
	{
		//LOG_DEBUG("VRL size for node " << getNode(i)->number << ": " << getNode(i)->elements->size());
		if( getNode(i)->elements->size() > 25 )
			LOG_DEBUG("Issue with node " << getNode(i)->number);
	}*/
}

void gcm::TetrMeshSecondOrder::build_first_order_border()
{
	// Prepare border data

	float solid_angle;
	float solid_angle_part;

	LOG_DEBUG("Looking for border nodes using angles");
	int nodeCount = 0;
	
	// Check border using solid angle comparation with 4*PI

	ElasticNode* node;
	//for( int i = 0; i < nodesNumber; i++ ) {
	for( MapIter itr = nodesMap.begin(); itr != nodesMap.end(); ++itr ) {
		int i = itr->first;
		node = getNode(i);
		node->setIsBorder(false);
		if( /*node->isLocal() &&*/ node->isFirstOrder() )
		{
			solid_angle = 0;
			for(unsigned j = 0; j < node->elements->size(); j++)
			{
				solid_angle_part = get_solid_angle(i, node->elements->at(j));
				if( solid_angle_part < 0 )
					for(unsigned z = 0; z < node->elements->size(); z++)
						LOG_DEBUG("ELement: " << node->elements->at(z));
				assert(solid_angle_part >= 0);
				solid_angle += solid_angle_part;
			}
			if( fabs(4 * M_PI - solid_angle) > M_PI * EQUALITY_TOLERANCE ) {
				node->setIsBorder (true);
				nodeCount++;
			}
		}
	}
	LOG_DEBUG("Found " << nodeCount << " border nodes");
	
	LOG_DEBUG("Constructing border triangles");
	
	int faceCount = 0;
	int number = 0;
	
	// FIXME TODO - make faces uniq!
	
	// Check all tetrs and construct border triangles
	//for(int i = 0; i < tetrsNumber; i++) {
	for( MapIter itr = tetrsMap.begin(); itr != tetrsMap.end(); ++itr ) {
		int i = itr->first;
		for( int j = 0; j < 4; j++ )
		{
			if( isTriangleBorder( getTetr(i)->verts ) )
				faceCount++;
			shiftArrayLeft( getTetr(i)->verts, 4 );
		}
	}
	
	LOG_DEBUG("Found " << faceCount << " border triangles");
	
	createTriangles(faceCount);
	
	//for(int i = 0; i < tetrsNumber; i++) {
	for( MapIter itr = tetrsMap.begin(); itr != tetrsMap.end(); ++itr ) {
		int i = itr->first;
		for( int j = 0; j < 4; j++ )
		{
			if( isTriangleBorder( getTetr(i)->verts ) )
			{
				*getTriangle(number) = createBorderTriangle( getTetr(i)->verts, number );
				number++;
			}
			shiftArrayLeft( getTetr(i)->verts, 4 );
		}
	}
	
	assert( number == faceCount );
	LOG_DEBUG("Created " << faceNumber << " triangles");
}

void gcm::TetrMeshSecondOrder::generateSecondOrderBorder()
{
	LOG_DEBUG("Faces: " << faceNumber);
	bool debug = false;
	
	LOG_DEBUG("Generating second order border");
	IntPair combinations[3];
	combinations[0] = make_pair(0,1);
	combinations[1] = make_pair(0,2);
	combinations[2] = make_pair(1,2);
	
	int v1, v2, ind;
	int v1pos, v2pos, l;
	int curTInd;
	int minI, maxI;
	for( int i = 0; i < faceNumber; i++)
	{
		for( int j = 0; j < 3; j++ )
		{
			v1 = getTriangle(i)->verts[ combinations[j].first ];
			v2 = getTriangle(i)->verts[ combinations[j].second ];
			
			//if( ( v1 == 0 && v2 == 420 ) || ( v1 == 420 && v2 == 0 ) )
			//	debug = true;
			
			ind = -1;
			l = getNode(v1)->elements->size();
			for( int k = 0; k < l; k++ )
			{
				v1pos = -1;
				v2pos = -1;
				curTInd = getNode(v1)->elements->at(k);
				for(int z = 0; z < 4; z++)
				{
					if( getTetr(curTInd)->verts[z] == v1 )
						v1pos = z;
					if( getTetr(curTInd)->verts[z] == v2 )
						v2pos = z;
				}
				if( v1pos >= 0 && v2pos >= 0 )
				{
					minI = (v1pos > v2pos ? v2pos : v1pos);
					maxI = (v1pos > v2pos ? v1pos : v2pos);
					if( minI == 0 && maxI == 1 )
						ind = getTetr2(curTInd)->addVerts[0];
					else if( minI == 0 && maxI == 2 )
						ind = getTetr2(curTInd)->addVerts[1];
					else if( minI == 0 && maxI == 3 )
						ind = getTetr2(curTInd)->addVerts[2];
					else if( minI == 1 && maxI == 2 )
						ind = getTetr2(curTInd)->addVerts[3];
					else if( minI == 1 && maxI == 3 )
						ind = getTetr2(curTInd)->addVerts[4];
					else if( minI == 2 && maxI == 3 )
						ind = getTetr2(curTInd)->addVerts[5];
					if( debug )
					{
						LOG_DEBUG("Tetr: " << curTInd << " Verts: " << v1 << " " << v2);
						LOG_DEBUG("Found origin for test node");
						LOG_DEBUG("Positions: " << v1pos << " " << v2pos);
						LOG_DEBUG("Index: " << ind);
						LOG_DEBUG("Node: " << *getNode(ind));
					}
					break;
				}
			}
			
			assert( ind != -1 );
			assert( getNode(ind)->isSecondOrder() );
			getTriangle2(i)->addVerts[j] = ind;
			getNode(ind)->setIsBorder(true);
		}
	}
	
	LOG_DEBUG( "Second order border generated" );
}

void gcm::TetrMeshSecondOrder::build_surface_reverse_lookups()
{
	LOG_DEBUG("Building surface reverse lookups for second order mesh");

	// Init vectors for "reverse lookups" of border triangles current node is a member of.
	//for(int i = 0; i < nodesNumber; i++) { 
	for( MapIter itr = nodesMap.begin(); itr != nodesMap.end(); ++itr ) {
		int i = itr->first;
		getNode(i)->border_elements->clear();
		//delete getNode(i)->border_elements;
		//getNode(i)->border_elements = new vector<int>; 
	}

	// Go through all the triangles and push to data of nodes the number of this triangle
	for(int i = 0; i < faceNumber; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			int nodeInd = getTriangle(i)->verts[j];
			assert( getNode(nodeInd)->isFirstOrder() );
			getNode(nodeInd)->border_elements->push_back(i);
		}
		for(int j = 0; j < 3; j++)
		{
			int nodeInd = getTriangle2(i)->addVerts[j];
			assert( getNode(nodeInd)->isSecondOrder() );
			getNode(nodeInd)->border_elements->push_back(i);
		}
	}
	
	for( MapIter itr = nodesMap.begin(); itr != nodesMap.end(); ++itr ) {
		ElasticNode* node = getNode(itr->first);
		int num = node->border_elements->size();
		if( node->isBorder() && num <= 0 )
			LOG_WARN("Border node is not a part of faces. Node: " << *node);
	}
}

void gcm::TetrMeshSecondOrder::move_coords(float tau)
{
	gcm::TetrMeshFirstOrder::move_coords(tau);
	mesh_min_h *= 0.5;
}

void gcm::TetrMeshSecondOrder::fillSecondOrderNode(ElasticNode* newNode, int nodeIdx1, int nodeIdx2)
{
	ElasticNode* node1 = getNode(nodeIdx1);
	ElasticNode* node2 = getNode(nodeIdx2);
	
	for( int i = 0; i < 3; i++ )
		newNode->coords[i] = ( node1->coords[i] + node2->coords[i] ) * 0.5;
	
	for(int i = 0; i < 9; i++)
		newNode->values[i] = ( node1->values[i] + node2->values[i] ) * 0.5;
	
	for( int i = 0; i < 3; i++ )
		newNode->elasticRheologyProperties[i] = 
				( node1->elasticRheologyProperties[i] + node2->elasticRheologyProperties[i] ) * 0.5;
	
	newNode->setPlacement(Local);
	newNode->setOrder( SecondOrder );
}

int gcm::TetrMeshSecondOrder::countSecondOrderNodes(TetrMeshFirstOrder* src)
{
	assert( src != NULL );
	LOG_DEBUG( "Counting additional nodes" );
	
	int firstOrderNodesCount = src->getNodesNumber();

	IntPair combinations[6];
	combinations[0] = make_pair(0,1);
	combinations[1] = make_pair(0,2);
	combinations[2] = make_pair(0,3);
	combinations[3] = make_pair(1,2);
	combinations[4] = make_pair(1,3);
	combinations[5] = make_pair(2,3);
	
	int minFirstOrderNum = src->getNodeByLocalIndex(0)->number;
	int maxFirstOrderNum = src->getNodeByLocalIndex(0)->number;
	for( int i = 0; i < firstOrderNodesCount; i++ )
	{
		int num = src->getNodeByLocalIndex(i)->number;
		if( num > maxFirstOrderNum )
			maxFirstOrderNum = num;
		if( num < minFirstOrderNum )
			minFirstOrderNum = num;
	}
	int firstOrderLength = maxFirstOrderNum - minFirstOrderNum + 1;
	//LOG_DEBUG("Min: " << minFirstOrderNum << " Max: " << maxFirstOrderNum << " Len: " << firstOrderLength);
	
	vector<IntPair>** processed = new vector<IntPair>*[firstOrderLength];
	for(int i = 0; i < firstOrderLength; i++)
		processed[i] = new vector<IntPair>;

	int secondOrderNodesCount = 0;
	int v1, v2;
	int ind;
	for( int i = 0; i < src->getTetrsNumber(); i++)
	{
		TetrFirstOrder* tetr = src->getTetrByLocalIndex(i);
		//if( body->getEngine()->getRank() == 1 )
		//	LOG_DEBUG("Tetr " << i << " Num: " << tetr->number);
		for( int j = 0; j < 6; j++ )
		{
			v1 = tetr->verts[ combinations[j].first ];
			v2 = tetr->verts[ combinations[j].second ];
			//if( body->getEngine()->getRank() == 1 )
			//	LOG_DEBUG("V1 = " << v1 << " V2 = " << v2);
			ind = -1;

			for( unsigned int z = 0; z < processed[v1 - minFirstOrderNum]->size(); z++ )
				if( (processed[v1 - minFirstOrderNum]->at(z)).second == v2 )
					ind = (processed[v1 - minFirstOrderNum]->at(z)).first;

			if( ind == -1 )
			{
				IntPair in;
				in.first = firstOrderNodesCount + secondOrderNodesCount;
				in.second = v2;
				processed[v1 - minFirstOrderNum]->push_back(in);
				in.second = v1;
				processed[v2 - minFirstOrderNum]->push_back(in);
				secondOrderNodesCount++;
			}
		}
	}

	for( int i = 0; i < firstOrderLength; i++ )
		delete processed[i];
	delete[] processed;
	
	LOG_DEBUG( "We are going to create " << secondOrderNodesCount << " second order nodes" );
	
	return secondOrderNodesCount;
}

void gcm::TetrMeshSecondOrder::generateSecondOrderNodes()
{
	ElasticNode node;
	
	if( secondOrderNodesAreGenerated )
		return;
	
	for( int i = 0; i < firstOrderNodesNumber; i++)
		getNode(i)->setOrder( FirstOrder );
	
	LOG_DEBUG( "Creating second order nodes" );
	LOG_DEBUG( "Number of first order nodes: " << nodesNumber );
	
	IntPair combinations[6];
	combinations[0] = make_pair(0,1);
	combinations[1] = make_pair(0,2);
	combinations[2] = make_pair(0,3);
	combinations[3] = make_pair(1,2);
	combinations[4] = make_pair(1,3);
	combinations[5] = make_pair(2,3);
	
	vector<IntPair>** processed = new vector<IntPair>*[firstOrderNodesNumber];
	for(int i = 0; i < firstOrderNodesNumber; i++)
		processed[i] = new vector<IntPair>;
	
	int secondOrderNodesCount = 0;
	int v1, v2;
	int ind;
	//for( int i = 0; i < tetrsNumber; i++) {
	for( MapIter itr = tetrsMap.begin(); itr != tetrsMap.end(); ++itr ) {
		int i = itr->first;
		for( int j = 0; j < 6; j++ )
		{
			v1 = getTetr(i)->verts[ combinations[j].first ];
			v2 = getTetr(i)->verts[ combinations[j].second ];
			ind = -1;

			for( unsigned int z = 0; z < processed[v1]->size(); z++ )
				if( (processed[v1]->at(z)).second == v2 )
					ind = (processed[v1]->at(z)).first;

			if( ind == -1 )
			{
				ind = firstOrderNodesNumber + secondOrderNodesCount;
				fillSecondOrderNode( &node, v1, v2 );
				node.number = ind;
				addNode( &node );
				IntPair in;
				in.first = ind;
				in.second = v2;
				processed[v1]->push_back(in);
				in.second = v1;
				processed[v2]->push_back(in);
				secondOrderNodesCount++;
			}
			getTetr2(i)->addVerts[j] = ind;
		}
	}

	for( int i = 0; i < firstOrderNodesNumber; i++ )
		delete processed[i];
	delete[] processed;

	LOG_DEBUG( "Second order nodes created" );
	
	LOG_DEBUG( "Total number of nodes: " << nodesNumber );

	secondOrderNodesAreGenerated = true;
}