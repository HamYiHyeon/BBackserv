using UnityEngine;
using UnityEngine.UI;

public class KillCount : MonoBehaviour
{
    public static KillCount instance;

    private int kills = 0;
    public Text killCountText;

    void Awake()
    {
        if (instance == null)
        {
            instance = this;
        }
        else
        {
            Destroy(gameObject);
        }
    }

    void Start()
    {
        UpdateKillCountText();
    }

    public void AddKill(int value)
    {
        kills += value;
        UpdateKillCountText();
    }

    private void UpdateKillCountText()
    {
        killCountText.text = "처치한 적: " + kills;
    }
}
