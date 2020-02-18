function quitBox(cmd)
{
    if (cmd=='quit')
    {
        open(location, '_self').close();
    }
    return false;
}
