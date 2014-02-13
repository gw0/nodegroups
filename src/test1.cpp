/**
 * nodegroups - Test1
 *
 * @author  gw0 [http://gw.tnode.com/] <gw.2014@tnode.com>
 * @version (see group_h_VERSION)
 */

#include "Snap.h"
#include "group.h"


void test_lovro(TGroupST& G, const PUNGraph& Graph, const TIntStrH& NIdLabelH, const TStr& LovroSStr, const TStr& LovroTStr) {
  TStr CurStr, TmpStr;

  // Prepare inverse mapping (till first space)
  TStrIntH LabelNIdH;
  for (TIntStrH::TIter I = NIdLabelH.BegI(); I != NIdLabelH.EndI(); I++) {
    I.GetDat().SplitOnCh(CurStr, ' ', TmpStr);
    LabelNIdH.AddDat(CurStr, I.GetKey());
  }

  // Populate group S (match without spaces)
  printf("\nGroup S:\n");
  TStrV LovroSStrV;
  LovroSStr.SplitOnAllCh(',', LovroSStrV);
  for (TStrV::TIter I = LovroSStrV.BegI(); I != LovroSStrV.EndI(); I++) {
    CurStr = *I;
    CurStr.DelChAll(' ');

    G.SubSNIdV.AddMerged(LabelNIdH.GetDat(CurStr));
    printf("%d\t%s\t%s\n", LabelNIdH.GetDat(CurStr).Val, CurStr.CStr(), NIdLabelH.GetDat(LabelNIdH.GetDat(CurStr).Val).CStr());
  }

  // Populate group T (match without spaces)
  printf("\nGroup T:\n");
  TStrV LovroTStrV;
  LovroTStr.SplitOnAllCh(',', LovroTStrV);
  for (TStrV::TIter I = LovroTStrV.BegI(); I != LovroTStrV.EndI(); I++) {
    CurStr = *I;
    CurStr.DelChAll(' ');

    G.SubTNIdV.AddMerged(LabelNIdH.GetDat(CurStr));
    printf("%d\t%s\t%s\n", LabelNIdH.GetDat(CurStr).Val, CurStr.CStr(), NIdLabelH.GetDat(LabelNIdH.GetDat(CurStr).Val).CStr());
  }

  // Populate results of node group extraction
  G.RecomputeAll(Graph, G.SubSNIdV, G.SubTNIdV);

  // Recompute on corresponding Erdos-Renyi random graphs
  TGroupST R;
  R.W = GroupExtractAvgRndGnm(R, G.N, G.M);

  // Print status
  printf("\n");
  printf("foo %s\n", G.GetStr().CStr());
  printf("  r %s\n", R.GetStr().CStr());
}


/**
 * Console application entry point
 */
int main(int argc, char* argv[]) {
  // Header
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs(TStr::Fmt("test1. Build: %.2f, %s, %s. Time: %s", group_h_VERSION, __TIME__, __DATE__, TExeTm::GetCurTm()), 1);
  TExeTm ExeTm;
  Try

  // Parameters
  const TStr PrefixFNm = Env.GetIfArgPrefixStr("-o:", "graph", "Input and output file name prefix (can be overriden)");
  const TStr InFNm = Env.GetIfArgPrefixStr("-i:", PrefixFNm + ".edgelist", "Input graph edges (undirected edge per line)");
  const TStr LabelFNm = Env.GetIfArgPrefixStr("-l:", PrefixFNm + ".labels", "Optional input node labels (node ID, node label)");
  const TStr OutFNm = Env.GetIfArgPrefixStr("-og:", PrefixFNm + ".groups", "Output group assignments (for S and T)");

  // Input
  PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(InFNm, false);
  TIntStrH NIdLabelH;
  if (TFile::Exists(LabelFNm)) {  // optional labels
    TSsParser Ss(LabelFNm, ssfTabSep);
    while (Ss.Next()) {
      if (Ss.GetFlds() > 0) {
        NIdLabelH.AddDat(Ss.GetInt(0), Ss.GetFld(1));
      }
    }
  }

  // Test1 "./test1 -o:../data_nets/football"
  TGroupSTV GroupV;
  TGroupST G = {};
  TStr LovroSStr = "Florida State,North Carolina,Virginia,Duke,Georgia Tech,Clemson,Maryland,North Carolina State,Wake Forest";
  TStr LovroTStr = "Florida State,North Carolina,Virginia,Duke,Clemson,Maryland,Georgia Tech,North Carolina State,Wake Forest";
  test_lovro(G, Graph, NIdLabelH, LovroSStr, LovroTStr);
  GroupV.Add(G);

  // Output
  FILE *F = fopen(OutFNm.CStr(), "wt");
  fprintf(F, "# Input: %s\n", InFNm.CStr());
  fprintf(F, "# Nodes: %d    Edges: %d\n", GroupV[0].N, GroupV[0].M);
  fprintf(F, "# Groups: %d\n", GroupV.Len());
  for (int j = 0; j < GroupV.Len(); ++j) {
    TGroupST& G = GroupV[j];
    fprintf(F, "#   %-3d %s\n", j, G.GetStr().CStr());
  }
  fprintf(F, "# NId\tGroupS\tGroupT\tNLabel\n");
  for (int j = 0; j < GroupV.Len(); ++j) {
    for (int i = 0; i < GroupV[j].SubSNIdV.Len(); ++i) {
      int NId = GroupV[j].SubSNIdV[i].Val;
      TStr NIdLabel = TStr("-");
      if (NIdLabelH.IsKey(NId))
        NIdLabel = NIdLabelH.GetDat(NId);

      if (GroupV[j].SubTNIdV.IsInBin(NId)) {  // in S and T
        fprintf(F, "%d\t%d\t%d\t%s\n", NId, j, j, NIdLabel.CStr());
      } else {  // in S, but not T
        fprintf(F, "%d\t%d\t-1\t%s\n", NId, j, NIdLabel.CStr());
      }
    }
    for (int i = 0; i < GroupV[j].SubTNIdV.Len(); ++i) {
      int NId = GroupV[j].SubTNIdV[i].Val;
      TStr NIdLabel = TStr("-");
      if (NIdLabelH.IsKey(NId))
        NIdLabel = NIdLabelH.GetDat(NId);

      if (!GroupV[j].SubSNIdV.IsInBin(NId)) {  // in T, but not S
        fprintf(F, "%d\t-1\t%d\t%s\n", NId, j, NIdLabel.CStr());
      }
    }
  }
  fclose(F);

  // Footer
  Catch
  printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
  return 0;
}