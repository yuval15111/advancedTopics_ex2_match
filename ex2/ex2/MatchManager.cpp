#include "MatchManager.h"
#include "AlgorithmRegistration.h"

void MatchManager::createGameManagers()
{
	AlgorithmRegistrar registrar = AlgorithmRegistrar::getInstance();
	auto algorithms = registrar.getAlgoFactoryVec();
	cout << "MatchManager before the for: " << algorithms.size() << endl;
	for (auto& algorithm : algorithms) {

		cout << "MatchManager inside for: " << m_name << "max steps: " << m_maxSteps << endl;
		 m_moveListVector.push_back(GameManager(m_name, m_maxSteps, m_rowsNum, m_colsNum,
			m_board, m_playerLocation, m_endLocation, move(algorithm())).play());
	}
}

AlgorithmRegistrar AlgorithmRegistrar::instance;