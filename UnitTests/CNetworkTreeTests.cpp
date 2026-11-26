/* Copyright (C) 2014-2026 Stefan-Mihai MOGA
This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// CNetworkTreeTests.cpp : unit tests for CNetworkTree
//

#include "stdafx.h"
#include "TestHelpers.h"

#include <initializer_list>
#include <iterator>
#include <set>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	namespace
	{
		/**
		 * Gives the tests control over the otherwise protected key lists and reports the destruction of every node,
		 * which is the only visible effect of DeleteTree and of the destructor.
		 */
		class CTestTree : public CNetworkTree
		{
		public:
			explicit CTestTree(int* pnDestroyed = nullptr) : m_pnDestroyed(pnDestroyed)
			{
			}

			~CTestTree()
			{
				if (m_pnDestroyed != nullptr)
					++(*m_pnDestroyed);
			}

			/// Appends a key together with the path derived from it. The keys of a node must be added in ascending order.
			void AddKey(int nKey)
			{
				m_listKey.Add(nKey);
				m_listCode.Add(PathOf(nKey));
				SetSize(GetSize() + 1);
			}

			/// The path that AddKey stores for a key.
			static CString PathOf(int nKey)
			{
				CString strFilePath;
				strFilePath.Format(_T("node-%d.dat"), nKey);
				return strFilePath;
			}

		private:
			int* m_pnDestroyed;
		};

		/// Creates a node holding the given ascending keys. The caller has to hand it over to a parent right away, which then owns it.
		CTestTree* NewNode(int* pnDestroyed, std::initializer_list<int> arrKeys, bool bLeaf)
		{
			CTestTree* pNode = new CTestTree(pnDestroyed);
			for (const int nKey : arrKeys)
				pNode->AddKey(nKey);
			pNode->SetLeaf(bLeaf);
			return pNode;
		}

		/// The keys of the tree that BuildThreeLevelTree creates.
		const int arrThreeLevelKeys[] = { 10, 20, 40, 60, 80, 100, 120, 140, 150, 160, 180, 200, 210, 230, 260, 300 };
		/// The number of nodes of the tree that BuildThreeLevelTree creates, including the root.
		const int nThreeLevelNodes = 11;

		/**
		 * Turns the root into the top of a balanced tree with three levels, the leaves being all on the third one:
		 *  - root:    [100 200]
		 *  - link 0:  [40]       with the leaves [10 20] and [60 80]
		 *  - link 1:  [140 160]  with the leaves [120], [150] and [180]
		 *  - link 2:  [260]      with the leaves [210 230] and [300]
		 */
		void BuildThreeLevelTree(CTestTree& root, int* pnDestroyed)
		{
			root.AddKey(100);
			root.AddKey(200);
			root.SetLeaf(false);

			CTestTree* pLeft = NewNode(pnDestroyed, { 40 }, false);
			pLeft->SetLink(0, NewNode(pnDestroyed, { 10, 20 }, true));
			pLeft->SetLink(1, NewNode(pnDestroyed, { 60, 80 }, true));
			root.SetLink(0, pLeft);

			CTestTree* pMiddle = NewNode(pnDestroyed, { 140, 160 }, false);
			pMiddle->SetLink(0, NewNode(pnDestroyed, { 120 }, true));
			pMiddle->SetLink(1, NewNode(pnDestroyed, { 150 }, true));
			pMiddle->SetLink(2, NewNode(pnDestroyed, { 180 }, true));
			root.SetLink(1, pMiddle);

			CTestTree* pRight = NewNode(pnDestroyed, { 260 }, false);
			pRight->SetLink(0, NewNode(pnDestroyed, { 210, 230 }, true));
			pRight->SetLink(1, NewNode(pnDestroyed, { 300 }, true));
			root.SetLink(2, pRight);
		}

		/// Turns the root into a full node with BALANCED_TREE_ORDER keys and a one-key leaf below each of its BALANCED_TREE_ORDER + 1 links.
		void BuildFullNodeTree(CTestTree& root, int* pnDestroyed)
		{
			static_assert(BALANCED_TREE_ORDER == 4, "The keys of this helper are written for a tree of order 4");

			for (const int nKey : { 100, 200, 300, 400 })
				root.AddKey(nKey);
			root.SetLeaf(false);

			int nLink = 0;
			for (const int nKey : { 50, 150, 250, 350, 450 })
				root.SetLink(nLink++, NewNode(pnDestroyed, { nKey }, true));
		}

		/// Fails the test unless the key is found with the path that AddKey stored for it.
		void ExpectFound(CNetworkTree& tree, int nKey)
		{
			CString strMessage;
			strMessage.Format(_T("Key %d"), nKey);
			CString strFilePath(_T("untouched"));

			Assert::IsTrue(tree.SearchNode(nKey, strFilePath), strMessage);
			Assert::AreEqual(CTestTree::PathOf(nKey), strFilePath, strMessage);
		}

		/// Fails the test unless the key is not found and the output path is left alone.
		void ExpectMissing(CNetworkTree& tree, int nKey)
		{
			CString strMessage;
			strMessage.Format(_T("Key %d"), nKey);
			CString strFilePath(_T("untouched"));

			Assert::IsFalse(tree.SearchNode(nKey, strFilePath), strMessage);
			Assert::AreEqual(CString(_T("untouched")), strFilePath, strMessage);
		}
	}

	TEST_CLASS(CNetworkTreeTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		//----------------------------------------------------------------------
		// Construction and run-time type information
		//----------------------------------------------------------------------

		TEST_METHOD(Constructor_NewTree_IsAnEmptyLeafThatIsItsOwnRoot)
		{
			CNetworkTree tree;

			Assert::IsTrue(tree.GetLeaf());
			Assert::AreEqual(0u, tree.GetSize());
			Assert::IsTrue(tree.GetRoot() == &tree);
		}

		TEST_METHOD(Constructor_NewTree_HasNoChildren)
		{
			CNetworkTree tree;

			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				Assert::IsTrue(tree.GetLink(nIndex) == nullptr, L"A new tree must not have children");
		}

		TEST_METHOD(Constructor_NewTree_HasNoFileListAndNoRootFolder)
		{
			CNetworkTree tree;

			Assert::IsTrue(tree.GetFileList().IsEmpty());
			Assert::IsTrue(tree.GetRootFolder().IsEmpty());
		}

		TEST_METHOD(IsKindOf_Tree_MatchesItselfAndItsBaseClasses)
		{
			CNetworkTree tree;

			Assert::IsTrue(tree.IsKindOf(RUNTIME_CLASS(CNetworkTree)));
			Assert::IsTrue(tree.IsKindOf(RUNTIME_CLASS(CNetworkBase)));
			Assert::IsFalse(tree.IsKindOf(RUNTIME_CLASS(CNetworkHash)));
		}

		TEST_METHOD(Tree_InheritedHelpers_AreAvailable)
		{
			CNetworkTree tree;

			Assert::AreEqual(CString(_T("000A")), tree.EncodeNetworkID(10));
			Assert::IsTrue(tree.IsValidCode(CString(_T("000A"))));
		}

		//----------------------------------------------------------------------
		// Accessors
		//----------------------------------------------------------------------

		TEST_METHOD(SetLeaf_Value_IsReturnedByGetLeaf)
		{
			CNetworkTree tree;

			tree.SetLeaf(false);
			Assert::IsFalse(tree.GetLeaf());

			tree.SetLeaf(true);
			Assert::IsTrue(tree.GetLeaf());
		}

		TEST_METHOD(SetSize_Value_IsReturnedByGetSize)
		{
			CNetworkTree tree;

			tree.SetSize(3);
			Assert::AreEqual(3u, tree.GetSize());

			tree.SetSize(0);
			Assert::AreEqual(0u, tree.GetSize());
		}

		TEST_METHOD(SetRoot_Pointer_IsReturnedByGetRoot)
		{
			CNetworkTree tree;
			CNetworkTree other;

			tree.SetRoot(&other);
			Assert::IsTrue(tree.GetRoot() == &other);

			tree.SetRoot(nullptr);
			Assert::IsTrue(tree.GetRoot() == nullptr);
		}

		TEST_METHOD(SetLink_EverySlot_IsReturnedByGetLinkIndependently)
		{
			int nDestroyed = 0;
			CTestTree tree(&nDestroyed);
			CNetworkTree* arrChildren[BALANCED_TREE_ORDER + 1] = {};

			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
			{
				arrChildren[nIndex] = new CTestTree(&nDestroyed);
				tree.SetLink(nIndex, arrChildren[nIndex]);
			}

			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				Assert::IsTrue(tree.GetLink(nIndex) == arrChildren[nIndex], L"A link was changed by setting another one");
		}

		//----------------------------------------------------------------------
		// ImportData / ExportData (placeholders)
		//----------------------------------------------------------------------

		TEST_METHOD(ImportData_Placeholder_ReturnsTrueAndDoesNotTouchTheFileSystem)
		{
			TestHelpers::CTempFolder folder;
			CNetworkTree tree;
			tree.SetFileList(folder.GetEntry(_T("files.txt")));

			Assert::IsTrue(tree.ImportData());

			Assert::IsFalse(TestHelpers::PathExists(folder.GetEntry(_T("files.txt"))));
			Assert::AreEqual(0u, tree.GetSize());
		}

		TEST_METHOD(ExportData_Placeholder_ReturnsTrueAndDoesNotTouchTheFileSystem)
		{
			TestHelpers::CTempFolder folder;
			CNetworkTree tree;
			tree.SetFileList(folder.GetEntry(_T("files.txt")));

			Assert::IsTrue(tree.ExportData());

			Assert::IsFalse(TestHelpers::PathExists(folder.GetEntry(_T("files.txt"))));
			Assert::AreEqual(0u, tree.GetSize());
		}

		TEST_METHOD(ImportAndExportData_WithoutFileList_StillReturnTrue)
		{
			CNetworkTree tree;

			Assert::IsTrue(tree.ImportData());
			Assert::IsTrue(tree.ExportData());
		}

		//----------------------------------------------------------------------
		// CreateTree
		//----------------------------------------------------------------------

		TEST_METHOD(CreateTree_AfterTheStateWasChanged_RestoresTheInitialState)
		{
			CTestTree tree;
			CTestTree other;
			tree.SetLeaf(false);
			tree.SetSize(3);
			tree.SetRoot(&other);

			Assert::IsTrue(tree.CreateTree());

			Assert::IsTrue(tree.GetLeaf());
			Assert::AreEqual(0u, tree.GetSize());
			Assert::IsTrue(tree.GetRoot() == &tree);
			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				Assert::IsTrue(tree.GetLink(nIndex) == nullptr, L"CreateTree must clear every link");
		}

		TEST_METHOD(CreateTree_Twice_IsHarmless)
		{
			CTestTree tree;

			Assert::IsTrue(tree.CreateTree());
			Assert::IsTrue(tree.CreateTree());

			Assert::IsTrue(tree.GetLeaf());
			Assert::AreEqual(0u, tree.GetSize());
		}

		//----------------------------------------------------------------------
		// DeleteTree and the destructor
		//----------------------------------------------------------------------

		TEST_METHOD(DeleteTree_Leaf_ReturnsTrueAndDestroysNothing)
		{
			int nDestroyed = 0;
			CTestTree leaf(&nDestroyed);

			Assert::IsTrue(leaf.DeleteTree());

			Assert::AreEqual(0, nDestroyed);
		}

		TEST_METHOD(DeleteTree_AllLinkSlotsUsed_DestroysEveryChild)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			root.SetLeaf(false);
			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				root.SetLink(nIndex, new CTestTree(&nDestroyed));

			Assert::IsTrue(root.DeleteTree());

			Assert::AreEqual(BALANCED_TREE_ORDER + 1, nDestroyed);
			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				Assert::IsTrue(root.GetLink(nIndex) == nullptr, L"DeleteTree must clear the links of the destroyed children");
		}

		TEST_METHOD(DeleteTree_SparseLinks_DestroysOnlyTheLinkedChildren)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			root.SetLeaf(false);
			root.SetLink(1, new CTestTree(&nDestroyed));
			root.SetLink(BALANCED_TREE_ORDER, new CTestTree(&nDestroyed));

			Assert::IsTrue(root.DeleteTree());

			Assert::AreEqual(2, nDestroyed);
			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				Assert::IsTrue(root.GetLink(nIndex) == nullptr);
		}

		TEST_METHOD(DeleteTree_ThreeLevelTree_DestroysAllDescendantsButNotTheNodeItself)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			BuildThreeLevelTree(root, &nDestroyed);

			Assert::IsTrue(root.DeleteTree());

			Assert::AreEqual(nThreeLevelNodes - 1, nDestroyed);
			for (int nIndex = 0; nIndex <= BALANCED_TREE_ORDER; nIndex++)
				Assert::IsTrue(root.GetLink(nIndex) == nullptr);
			Assert::AreEqual(2u, root.GetSize());
		}

		TEST_METHOD(DeleteTree_CalledTwice_DestroysEachNodeOnlyOnce)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			BuildThreeLevelTree(root, &nDestroyed);

			Assert::IsTrue(root.DeleteTree());
			Assert::IsTrue(root.DeleteTree());

			Assert::AreEqual(nThreeLevelNodes - 1, nDestroyed);
		}

		TEST_METHOD(Destructor_NodeWithDescendants_DestroysThemAll)
		{
			int nDestroyed = 0;
			{
				CTestTree root(&nDestroyed);
				BuildThreeLevelTree(root, &nDestroyed);
				Assert::AreEqual(0, nDestroyed);
			}

			Assert::AreEqual(nThreeLevelNodes, nDestroyed);
		}

		TEST_METHOD(Destructor_AfterDeleteTree_DoesNotDestroyTheChildrenAgain)
		{
			int nDestroyed = 0;
			{
				CTestTree root(&nDestroyed);
				BuildThreeLevelTree(root, &nDestroyed);
				Assert::IsTrue(root.DeleteTree());
			}

			Assert::AreEqual(nThreeLevelNodes, nDestroyed);
		}

		TEST_METHOD(Destructor_ChildDeletedThroughItsBasePointer_RunsTheDerivedDestructor)
		{
			int nDestroyed = 0;
			CNetworkTree* pTree = new CTestTree(&nDestroyed);

			delete pTree;

			Assert::AreEqual(1, nDestroyed);
		}

		//----------------------------------------------------------------------
		// SearchNode in a single node
		//----------------------------------------------------------------------

		TEST_METHOD(SearchNode_EmptyTree_ReturnsFalseAndLeavesThePathAlone)
		{
			CTestTree tree;

			ExpectMissing(tree, 0);
			ExpectMissing(tree, 1);
			ExpectMissing(tree, -1);
			ExpectMissing(tree, MAX_RANDOM_DATA);
		}

		TEST_METHOD(SearchNode_LeafKeys_ReturnTheirOwnPaths)
		{
			CTestTree leaf;
			for (const int nKey : { 10, 20, 30, 40 })
				leaf.AddKey(nKey);

			ExpectFound(leaf, 10);
			ExpectFound(leaf, 20);
			ExpectFound(leaf, 30);
			ExpectFound(leaf, 40);
		}

		TEST_METHOD(SearchNode_KeyBelowTheSmallestOfALeaf_ReturnsFalse)
		{
			CTestTree leaf;
			for (const int nKey : { 10, 20, 30, 40 })
				leaf.AddKey(nKey);

			ExpectMissing(leaf, 9);
			ExpectMissing(leaf, 0);
			ExpectMissing(leaf, -10);
		}

		TEST_METHOD(SearchNode_KeyBetweenTheKeysOfALeaf_ReturnsFalse)
		{
			CTestTree leaf;
			for (const int nKey : { 10, 20, 30, 40 })
				leaf.AddKey(nKey);

			ExpectMissing(leaf, 11);
			ExpectMissing(leaf, 19);
			ExpectMissing(leaf, 21);
			ExpectMissing(leaf, 29);
			ExpectMissing(leaf, 31);
			ExpectMissing(leaf, 39);
		}

		TEST_METHOD(SearchNode_KeyAboveTheLargestOfALeaf_ReturnsFalse)
		{
			CTestTree leaf;
			for (const int nKey : { 10, 20, 30, 40 })
				leaf.AddKey(nKey);

			ExpectMissing(leaf, 41);
			ExpectMissing(leaf, MAX_RANDOM_DATA);
			ExpectMissing(leaf, INT_MAX);
		}

		TEST_METHOD(SearchNode_LeafWithSingleKey_FindsOnlyThatKey)
		{
			CTestTree leaf;
			leaf.AddKey(77);

			ExpectFound(leaf, 77);
			ExpectMissing(leaf, 76);
			ExpectMissing(leaf, 78);
		}

		TEST_METHOD(SearchNode_NegativeAndZeroKeys_AreComparedAsPlainIntegers)
		{
			CTestTree leaf;
			for (const int nKey : { -5, 0, 5 })
				leaf.AddKey(nKey);

			ExpectFound(leaf, -5);
			ExpectFound(leaf, 0);
			ExpectFound(leaf, 5);
			ExpectMissing(leaf, -6);
			ExpectMissing(leaf, -1);
			ExpectMissing(leaf, 1);
			ExpectMissing(leaf, 6);
		}

		TEST_METHOD(SearchNode_LargeLeaf_FindsEveryKeyAndRejectsEveryGap)
		{
			CTestTree leaf;
			const int nLargest = 2000;
			for (int nKey = 2; nKey <= nLargest; nKey += 2)
				leaf.AddKey(nKey);

			for (int nKey = 0; nKey <= nLargest + 1; nKey++)
			{
				if ((nKey >= 2) && (nKey <= nLargest) && (nKey % 2 == 0))
					ExpectFound(leaf, nKey);
				else
					ExpectMissing(leaf, nKey);
			}
		}

		//----------------------------------------------------------------------
		// SearchNode in trees of several levels
		//----------------------------------------------------------------------

		TEST_METHOD(SearchNode_TwoLevelTree_FindsTheKeysOfTheRootAndOfBothLeaves)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			root.AddKey(50);
			root.SetLeaf(false);
			root.SetLink(0, NewNode(&nDestroyed, { 10, 20, 30 }, true));
			root.SetLink(1, NewNode(&nDestroyed, { 60, 70, 80 }, true));

			for (const int nKey : { 10, 20, 30, 50, 60, 70, 80 })
				ExpectFound(root, nKey);
		}

		TEST_METHOD(SearchNode_TwoLevelTree_RejectsTheKeysThatAreNotStored)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			root.AddKey(50);
			root.SetLeaf(false);
			root.SetLink(0, NewNode(&nDestroyed, { 10, 20, 30 }, true));
			root.SetLink(1, NewNode(&nDestroyed, { 60, 70, 80 }, true));

			for (const int nKey : { -1, 5, 15, 25, 35, 49, 51, 55, 65, 75, 85, MAX_RANDOM_DATA })
				ExpectMissing(root, nKey);
		}

		TEST_METHOD(SearchNode_KeyStoredInTheNode_DoesNotDescendIntoTheChildren)
		{
			CTestTree root;
			root.AddKey(100);
			root.AddKey(200);
			root.SetLeaf(false);

			ExpectFound(root, 100);
			ExpectFound(root, 200);
		}

		TEST_METHOD(SearchNode_KeyStoredInAChild_ReturnsThePathOfTheChildKey)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			root.AddKey(100);
			root.SetLeaf(false);
			root.SetLink(0, NewNode(&nDestroyed, { 40 }, true));
			root.SetLink(1, NewNode(&nDestroyed, { 140 }, true));
			CString strFilePath;

			Assert::IsTrue(root.SearchNode(40, strFilePath));
			Assert::AreEqual(CString(_T("node-40.dat")), strFilePath);
			Assert::IsTrue(root.SearchNode(140, strFilePath));
			Assert::AreEqual(CString(_T("node-140.dat")), strFilePath);
		}

		TEST_METHOD(SearchNode_ThreeLevelTree_FindsExactlyTheStoredKeys)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			BuildThreeLevelTree(root, &nDestroyed);
			const std::set<int> setKeys(std::begin(arrThreeLevelKeys), std::end(arrThreeLevelKeys));

			for (int nKey = -5; nKey <= 320; nKey++)
			{
				if (setKeys.count(nKey) != 0)
					ExpectFound(root, nKey);
				else
					ExpectMissing(root, nKey);
			}
		}

		TEST_METHOD(SearchNode_FullNode_UsesTheLastLinkForTheLargestKeys)
		{
			int nDestroyed = 0;
			CTestTree root(&nDestroyed);
			BuildFullNodeTree(root, &nDestroyed);

			for (const int nKey : { 50, 100, 150, 200, 250, 300, 350, 400, 450 })
				ExpectFound(root, nKey);
			for (const int nKey : { 49, 51, 99, 101, 399, 401, 449, 451, MAX_RANDOM_DATA })
				ExpectMissing(root, nKey);
		}

		//----------------------------------------------------------------------
		// GenerateID
		//----------------------------------------------------------------------

		TEST_METHOD(GenerateID_EmptyTree_ReturnsAnIdInTheRandomRange)
		{
			CTestTree tree;
			srand(31);

			for (int nCall = 0; nCall < 1000; nCall++)
			{
				const int nId = tree.GenerateID();
				Assert::IsTrue((nId >= 1) && (nId <= MAX_RANDOM_DATA));
			}
		}

		TEST_METHOD(GenerateID_Call_DoesNotAddTheIdToTheTree)
		{
			CTestTree tree;

			const int nId = tree.GenerateID();

			Assert::AreEqual(0u, tree.GetSize());
			ExpectMissing(tree, nId);
		}

		TEST_METHOD(GenerateID_FirstCandidateAlreadyInTheTree_ReturnsTheNextCandidate)
		{
			CTestTree tree;
			srand(99);
			const int nTaken = tree.GetNextID();
			int nExpected = nTaken;
			while (nExpected == nTaken)
				nExpected = tree.GetNextID();
			tree.AddKey(nTaken);

			srand(99);

			Assert::AreEqual(nExpected, tree.GenerateID());
		}

		TEST_METHOD(GenerateID_CandidatesStoredInChildNodes_AreSkipped)
		{
			int nDestroyed = 0;
			CTestTree tree(&nDestroyed);
			srand(1234);
			const int nFirst = tree.GetNextID();
			tree.AddKey(nFirst + 1);
			tree.SetLeaf(false);
			tree.SetLink(0, NewNode(&nDestroyed, { nFirst }, true));
			tree.SetLink(1, NewNode(&nDestroyed, { nFirst + 2 }, true));
			const std::set<int> setStored = { nFirst, nFirst + 1, nFirst + 2 };
			int nExpected = nFirst;
			while (setStored.count(nExpected) != 0)
				nExpected = tree.GetNextID();

			srand(1234);

			Assert::AreEqual(nExpected, tree.GenerateID());
		}

		TEST_METHOD(GenerateID_TreeWithManyKeys_NeverReturnsAStoredKey)
		{
			CTestTree tree;
			std::set<int> setKeys;
			srand(7);
			while (setKeys.size() < 300)
				setKeys.insert(tree.GetNextID());
			for (const int nKey : setKeys)
				tree.AddKey(nKey);

			for (int nCall = 0; nCall < 1000; nCall++)
			{
				const int nId = tree.GenerateID();
				Assert::IsTrue(setKeys.count(nId) == 0, L"GenerateID returned a key that is already stored");
				Assert::IsTrue((nId >= 1) && (nId <= MAX_RANDOM_DATA));
			}
		}
	};
}
