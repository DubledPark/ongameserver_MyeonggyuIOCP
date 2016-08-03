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
	
	// ������ȣ�� ��ǥ���� ������ �ҷ��� �ܼ��� ��Ʈ���ĸ� �Ѱܼ� �ȵǰ�, interface�� �ʿ��ϴ�.
	//	���1: pathfinder�� ����ҷ��� pathfinder���� ������ interface�� ���� �׷����ڷᱸ���� ��ӹ޾Ƽ� ������ �־�� �Ѵ�.
	//	���2: �ʿ��� �Լ� ���� map< keytype, type, less >���� less�Լ��� �������ؼ� �޴°�ó�� �Ѵ�.
	//	���3: PathFinder�� ��ӹ޾Ƽ� �ʿ��� �Լ����� �������ؼ� ����Ѵ�.
	//		f(hueristic) method
	//		getNodeUniqueNumber()
	//	���4: GraphManager�� �Ѱ��༭ GraphManager���� �ʿ��� ������� �����ϰ� �Ѵ�.
	//		��ǥ������ �׷����������������(������ȣ�� ����ؼ�)
	//		�� ��ǥ���� �޸���ƽ�� ������
	//		���� ��ǥ���� �������� �̿��ϴ� ��� ����� ���� ������
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
		// �ϴ� �湮Ʈ���� ����� ��� ������ ���ϵ��带 �ϳ� �����Ѵ�.
		for( c = 0; c < 8; c++)		
		{
			if( BestNode->s_Child[c] == NULL )
				break;
		}
		// ���õ� ���ϵ��忡 ����ϰ�
		BestNode->s_Child[c] = Old;
		
		// g���� �����庸�� �۴ٸ�
		//	�� �̹� �湮������ �ִµ�, ���� �湮�� ���Լ����� �� �۴ٸ�
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
		// �׳�尡 Ŭ�����Ʈ�� �����ϴ°�?

		// ���ϵ忡 ��Ͻ�Ű��
		for( c = 0; c< 8; c++)
		{
			if ( BestNode->s_Child[c] == NULL )
				break;
		}
		
		BestNode->s_Child[c] = Old;
			
		// ���� �������Լ��� �� �۴ٸ�
		if ( g < Old->s_dw_g )
		{
			Old->s_pParent = BestNode;
			Old->s_dw_g = g;
			Old->s_f = g + Old->s_h;

			// ���� �湮�� ��带 �Ʒ��� �������Ѵ�?
			PropagateDown(Old);
		}
	}
	else
	{
		// �湮������ �����ٸ�
		Successor = new NODE();

		//bestnode�� ���ϵ�� �����Ѵ�.
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
	//�� �Լ��� �򰡰��� ���� Ʈ���� �������ϴ� �˰����̴�.
	//	����� Ŭ�����Ʈ���� ��尡 Ž���� ��쿡 ȣ��ȴ�.

	DWORD c, g;
	NODE *Child, *Father;
	
	g = Old->s_dw_g;
	for ( c = 0; c < 8; c++)
	{
		// �����ĵǾ���� old����� ���ϵ带 ã�´�.
		if ( (Child=Old->s_Child[c]) == NULL )
			break;

		// �����򰡰��� child�� �򰡰����� ������
		if ( g+1 < Child->s_dw_g)
		{
			// �ش����ϵ��� �򰡰��� �θ� �缳���Ѵ�.
			Child->s_dw_g = g+1;
			Child->s_f = Child->s_dw_g + Child->s_h;
			Child->s_pParent = Old;

			// �׸��� m_pStacklist�� �ϴ� �������ϵ带 �߰��Ѵ�.
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








