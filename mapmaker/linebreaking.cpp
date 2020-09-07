#include "linebreaking.h"


void breakLines(std::vector<int> &words, int maxLineWidth, std::vector<size_t> *breaks)
{
	if (words.size() > 0)
	{
		int lineLength = words[0];
		for (size_t i = 1; i < words.size(); ++i)
		{
			if (lineLength + words[i] > maxLineWidth)
			{
				breaks->push_back(i);
				lineLength = 0;
			}

			lineLength += words[i];
		}
	}
}


