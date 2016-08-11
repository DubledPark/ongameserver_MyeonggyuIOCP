#include "PathFinder.h"

PathFinder::PathFinder() : openList(), closeList(), usedList(), travTreeList()
{	
}

PathFinder::~PathFinder()
{
	Finit();	
}

void PathFinder::Finit()
{
	travTreeList.clear();
	openList.clear();
	closeList.clear();

	size_t usedsize = usedList.size();

	if(0 >= usedsize) return;

	tNODELISTITER iter = usedList.begin();
	size_t count = 0;
	for( ; count < usedsize && iter != usedList.end() ; ++count, ++iter)
	{
		NODE* node = *iter;
		if(node) delete node;
	}
	usedList.clear();
}

BOOL PathFinder::findPath(Position3D& start, Position3D& end, IGraphManager* igm, std::list<DWORD>& output)
{
	if(NULL == igm) return FALSE;

	instantGraphManager = igm;

	NODE* node = new NODE();
	usedList.push_back(node);

	node->s_dw_g = 0;
	node->s_h = instantGraphManager->calcHueristic(start, end);
	node->s_f = node->s_dw_g+node->s_h;
	
	// 고유번호와 좌표값을 얻어오게 할려면 단순한 스트럭쳐를 넘겨선 안되고, interface가 필요하다.
	//	방법1: pathfinder를 사용할려면 pathfinder에서 정의한 interface를 실제 그래프자료구조가 상속받아서 구현해 있어야 한다.
	//	방법2: 필요한 함수 몇몇만을 map< keytype, type, less >에서 less함수를 재정의해서 받는것처럼 한다.
	//	방법3: PathFinder를 상속받아서 필요한 함수들을 재정의해서 사용한다.
	//		f(hueristic) method
	//		getNodeUniqueNumber()
	//	방법4: GraphManager를 넘겨줘서 GraphManager에게 필요한 내용들을 질의하게 한다.
	//		좌표값으로 그래프노드정보얻어오기(고유번호를 비롯해서)
	//		두 좌표간의 휴리스틱값 얻어오기
	//		현재 좌표값을 기준으로 이웃하는 모든 노드의 정보 얻어오기
	node->s_dw_NodeNum = instantGraphManager->getUniqueNumber(start);
	node->s_pos = start;

	openList.push_front(node);
	DWORD count = 0;
	NODE* BestNode = NULL;

	DWORD dwDestNodeNum = instantGraphManager->getUniqueNumber(end);
	BOOL findPath = FALSE;

	while(false == openList.empty())
	{
		BestNode = ReturnBestNode();

		if(NULL == BestNode) break;

		if(dwDestNodeNum == BestNode->s_dw_NodeNum)
		{
			findPath = TRUE;
			break;
		}

		GenerateSuccessors(BestNode, end);
		count++;

	}

	if(findPath)
	{
		output.push_front(BestNode->s_dw_NodeNum);

		NODE* pp = BestNode->s_pParent;
		while(pp)
		{
			output.push_front(pp->s_dw_NodeNum);
			pp = pp->s_pParent;
		}
	}

	Finit();
	return findPath;
}

PathFinder::NODE* PathFinder::ReturnBestNode()
{
	NODE* tmp = NULL;

	if(openList.empty()) return NULL;

	tmp = openList.front();
	openList.pop_front();

	closeList.push_front(tmp);
	
	return tmp;
}

void PathFinder::GenerateSuccessors(NODE* BestNode, Position3D& end)
{
	tPosition3DLIST neighbors;

	instantGraphManager->gatherNeighborNodes(BestNode->s_pos, neighbors);
	Position3D child;

	for(tPosition3DLISTITER iter = neighbors.begin() ; iter != neighbors.end() ; ++iter)
	{
		child = *iter;
		GenerateSucc(BestNode, child, end);
	}
}


void PathFinder::GenerateSucc(NODE* BestNode, Position3D& childpos, Position3D& end)
{
	DWORD g,curNodeNum,c=0;
	NODE* Old;
	NODE* Successor;

	g = BestNode->s_dw_g+1;
	curNodeNum = instantGraphManager->getUniqueNumber(childpos);

	if( NULL != (Old = CheckOpen(curNodeNum)) )
	{
		// 일단 방문트리를 만들기 어디에 넣을지 차일드노드를 하나 선택한다.
		for( c = 0; c < 8; c++)		
		{
			if( BestNode->s_Child[c] == NULL )
				break;
		}
		// 선택된 차일드노드에 등록하고
		BestNode->s_Child[c] = Old;
		
		// g값이 현재노드보다 작다면
		//	즉 이미 방문한적이 있는데, 지금 방문한 평가함수값이 더 작다면
		//	
		if ( g < Old->s_dw_g )
		{
			Old->s_pParent = BestNode;
			Old->s_dw_g = g;
			Old->s_f = g + Old->s_h;
		}
	}
	else if( NULL != (Old = CheckClosed(curNodeNum)) )
	{
		// 그노드가 클로즈리스트에 존재하는가?

		// 차일드에 등록시키고
		for( c = 0; c< 8; c++)
		{
			if ( BestNode->s_Child[c] == NULL )
				break;
		}
		
		BestNode->s_Child[c] = Old;
			
		// 역시 현재평가함수가 더 작다면
		if ( g < Old->s_dw_g )
		{
			Old->s_pParent = BestNode;
			Old->s_dw_g = g;
			Old->s_f = g + Old->s_h;

			// 현재 방문한 노드를 아래로 재정렬한다?
			PropagateDown(Old);
		}
	}
	else
	{
		// 방문한적이 없었다면
		Successor = new NODE();

		//bestnode의 차일드로 설정한다.
		Successor->s_pParent = BestNode;
		Successor->s_dw_g = g;
		Successor->s_h = instantGraphManager->calcHueristic(childpos, end);
		Successor->s_f = g+Successor->s_h;
		Successor->s_dw_NodeNum = instantGraphManager->getUniqueNumber(childpos);
		Successor->s_pos = childpos;

		
		Insert(Successor);

		for( c =0; c < 8; c++)
			if ( BestNode->s_Child[c] == NULL )
				break;
		BestNode->s_Child[c] = Successor;
	}

}



void PathFinder::Insert(NODE* Successor)
{
	usedList.push_back(Successor);

	if(openList.empty())
	{
		openList.push_front(Successor);
		return;
	}

	int f = Successor->s_f;
	NODE* node = NULL;
	
	openList.push_back(Successor);
	openList.sort(PathLess);	
	/*
	tNODELISTITER iter = openList.begin();
	for(; iter != openList.end() ; ++iter)
	{
		node = *iter;
		if(node)
		{
			if( f >= node->s_f ) break;
		}
	}

	if(iter == openList.begin())
	{
		openList.push_front(Successor);
	}
	else if(iter == openList.end())
	{
		openList.push_back(Successor);
	}
	else
	{
		--iter;
		openList.insert(iter, Successor);
	}	
	*/
}


PathFinder::NODE* PathFinder::CheckOpen(DWORD nodeNum)
{
	NODE* tmp = NULL;
	if(openList.empty()) return NULL;

	for(tNODELISTITER iter = openList.begin() ; iter != openList.end() ; ++iter)
	{
		tmp = *iter;
		if(tmp)
		{
			if(nodeNum == tmp->s_dw_NodeNum)
				return tmp;
		}
		else return NULL;
	}

	return NULL;
}

PathFinder::NODE* PathFinder::CheckClosed(DWORD nodeNum)
{
	NODE* tmp = NULL;
	if(closeList.empty()) return NULL;

	for(tNODELISTITER iter = closeList.begin() ; iter != closeList.end() ; ++iter)
	{
		tmp = *iter;
		if(tmp)
		{
			if(nodeNum == tmp->s_dw_NodeNum)
				return tmp;
		}
		else return NULL;
	}

	return NULL;
}

void PathFinder::PropagateDown(NODE* Old)
{
	//이 함수는 평가값에 의해 트리를 재정렬하는 알고리즘이다.
	//	현재는 클로즈리스트에서 노드가 탐색된 경우에 호출된다.

	DWORD c, g;
	NODE *Child, *Father;
	
	g = Old->s_dw_g;
	for ( c = 0; c < 8; c++)
	{
		// 재정렬되어야할 old노드의 차일드를 찾는다.
		if ( (Child=Old->s_Child[c]) == NULL )
			break;

		// 현재평가값이 child의 평가값보다 작으면
		if ( g+1 < Child->s_dw_g)
		{
			// 해당차일드의 평가값과 부모를 재설정한다.
			Child->s_dw_g = g+1;
			Child->s_f = Child->s_dw_g + Child->s_h;
			Child->s_pParent = Old;

			// 그리고 m_pStacklist에 일단 현재차일드를 추가한다.
			Push(Child);
		}
	}
	
	while(travTreeList.size())
	{
		
		Father = Pop();
		for (c = 0; c<8; c++)
		{
			if ( (Child=Father->s_Child[c]) == NULL )
				break;
			if ( Father->s_dw_g+1 < Child->s_dw_g )
			{
				Child->s_dw_g = Father->s_dw_g+1;
				Child->s_f = Child->s_dw_g+Child->s_h;
				Child->s_pParent = Father;
				Push(Child);
			}
		}
	}
}


void PathFinder::Push(NODE* Node)
{
	travTreeList.push_front(Node);
}


PathFinder::NODE* PathFinder::Pop()
{
	if(travTreeList.empty()) return NULL;

	NODE* node = travTreeList.front();
	travTreeList.pop_front();
	return node;
}








