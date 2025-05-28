using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

public class MultiplayersBuildAndRun
{
    [MenuItem("Tools/Run Bultiplayer/2 Players")]
    static void PerfomWin64Build2()
    {
        PerfomWin64Build(2);
    }
    [MenuItem("Tools/Run Bultiplayer/3 Players")]
    static void PerfomWin64Build3()
    {
        PerfomWin64Build(3);
    }
    [MenuItem("Tools/Run Bultiplayer/4 Players")]
    static void PerfomWin64Build4()
    {
        PerfomWin64Build(4);
    }

    static void PerfomWin64Build(int playerCount)
    {
        EditorUserBuildSettings.SwitchActiveBuildTarget(
        BuildTargetGroup.Standalone, BuildTarget.StandaloneWindows);

        for (int i = 1; i <= playerCount; i++)
        {
            BuildPipeline.BuildPlayer(GetScenePathes(),
                "Builds/Win64/" + GetProjectName() + i.ToString() + "/" + GetProjectName() + i.ToString() + ".exe",
                BuildTarget.StandaloneWindows64, BuildOptions.AutoRunPlayer);
        }
    }

    static string GetProjectName()
    {
        string[] s = Application.dataPath.Split('/');
        return s[s.Length - 2];
    }

    static string[] GetScenePathes()
    {
        string[] scenes = new string[EditorBuildSettings.scenes.Length];
        
        for (int i = 0; i < scenes.Length; i++)
        {
            scenes[i] = EditorBuildSettings.scenes[i].path;
        }

        return scenes;
    }
}
